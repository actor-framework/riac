/******************************************************************************
 *                       ____    _    _____                                   *
 *                      / ___|  / \  |  ___|    C++                           *
 *                     | |     / _ \ | |_       Actor                         *
 *                     | |___ / ___ \|  _|      Framework                     *
 *                      \____/_/   \_|_|                                      *
 *                                                                            *
 * Copyright (C) 2011 - 2014                                                  *
 * Dominik Charousset <dominik.charousset (at) haw-hamburg.de>                *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENCE_ALTERNATIVE.       *
 *                                                                            *
 * If you did not receive a copy of the license files, see                    *
 * http://opensource.org/licenses/BSD-3-Clause and                            *
 * http://www.boost.org/LICENSE_1_0.txt.                                      *
 ******************************************************************************/

#include "caf/probe/init.hpp"

#include <cstring>
#include <fstream>
#include <iostream>

#include <unistd.h>

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "caf/probe/if.hpp"
#include "caf/probe_event/all.hpp"

#include "caf/detail/singletons.hpp"

namespace caf {
namespace probe {

namespace {

class fwd_hook : public io::hook {
 public:
  fwd_hook(probe_event::nexus_type uplink)
      : m_self(true),
        m_uplink(uplink),
        m_node(detail::singletons::get_node_id()) {
    // nop
  }
  template<class T, class... Ts>
  void transmit(Ts&&... args) {
    m_self->send(m_uplink, T{std::forward<Ts>(args)...});
  }
  void message_received_cb(const node_id& src, const actor_addr& from,
                           const actor_addr& dest, message_id mid,
                           const message& msg) override {
    transmit<probe_event::new_message>(from.node(), dest.node(), from.id(),
                                       dest.id(), msg);
    call_next<message_received>(src, from, dest, mid, msg);
  }

  void message_sent_cb(const actor_addr& from, const node_id& hop,
                       const actor_addr& dest, message_id mid,
                       const message& msg) override {
    if (dest == m_uplink) {
      // let's avoid endless recursion, shall we?
      return;
    }
    transmit<probe_event::new_message>(from.node(), dest.node(), from.id(),
                                       dest.id(), msg);
    call_next<message_sent>(from, hop, dest, mid, msg);
  }

  void message_forwarded_cb(const node_id& from, const node_id& dest,
                            const std::vector<char>* payload) override {
    // do nothing (yet)
    call_next<message_forwarded>(from, dest, payload);
  }

  void message_sending_failed_cb(const actor_addr& from, const actor_addr& dest,
                                 message_id mid,
                                 const message& payload) override {
    // do nothing (yet)
    call_next<message_sending_failed>(from, dest, mid, payload);
  }

  void message_forwarding_failed_cb(const node_id& from, const node_id& to,
                                    const std::vector<char>* payload) override {
    // do nothing (yet)
    call_next<message_forwarding_failed>(from, to, payload);
  }

  void actor_published_cb(const actor_addr& addr, uint16_t port) override {
    // do nothing (yet)
    call_next<actor_published>(addr, port);
  }

  void new_remote_actor_cb(const actor_addr& addr) override {
    // do nothing (yet)
    call_next<new_remote_actor>(addr);
  }

  void new_connection_established_cb(const node_id& dest) override {
    transmit<probe_event::new_route>(m_node, dest, true);
    call_next<new_connection_established>(dest);
  }

  void new_route_added_cb(const node_id& via, const node_id& dest) override {
    transmit<probe_event::new_route>(m_node, dest, false);
    call_next<new_route_added>(via, dest);
  }

  void invalid_message_received_cb(const node_id& source,
                                   const actor_addr& sender,
                                   actor_id invalid_dest, message_id mid,
                                   const message& msg) override {
    // do nothing (yet)
    call_next<invalid_message_received>(source, sender, invalid_dest, mid, msg);
  }

 private:
  scoped_actor m_self;
  probe_event::nexus_type m_uplink;
  node_id m_node;
};

struct probe_config {
  uint16_t port;
  std::string host;
  std::string config_file_path;
  inline probe_config() : port(0) { }
  inline bool valid() const {
    return !host.empty() && port != 0;
  }
};

//const char conf_file_arg[] = "--caf_probe_config_file=";
const char host_arg[] = "--caf-probe-host=";
const char port_arg[] = "--caf-probe-port=";

template<size_t Size>
bool is_substr(const char (&needle)[Size], const char* haystack) {
  // compare without null terminator
  if (strncmp(needle, haystack, Size - 1) == 0) {
    return true;
  }
  return false;
}

template<size_t Size>
size_t cstr_len(const char (&)[Size]) {
  return Size - 1;
}

void from_args(probe_config& conf, int argc, char** argv) {
  for (auto i = argv; i != argv + argc; ++i) {
    if (is_substr(host_arg, *i)) {
      conf.host.assign(*i + cstr_len(host_arg));
    } else if (is_substr(port_arg, *i)) {
      int p = std::stoi(*i + cstr_len(port_arg));
      conf.port = static_cast<uint16_t>(p);
    }
  }
}

} // namespace <anonymous>

std::string hostname() {
  char hostname[30];
  gethostname(hostname, sizeof(hostname));
  return std::string(hostname);
}

bool init(int argc, char** argv) {
  probe_event::announce_types();
  probe_config conf;
  from_args(conf, argc, argv);
  if (!conf.valid()) {
    return false;
  }
  auto uplink = io::typed_remote_actor<probe_event::nexus_type>(conf.host,
                                                                conf.port);
  io::middleman::instance()->add_hook<fwd_hook>(uplink);
  probe_event::node_info ni;
  ni.source_node = detail::singletons::get_node_id();
  auto interface_names = interface::ifnames();
  for (auto name : interface_names) {
    probe_event::interface_info ii;
    ii.source_node  = ni.source_node;
    ii.name         = name;
    ii.hw_addr      = interface::hw_addr(name);
    ii.ipv4_addr    = interface::ipv4_addr(name);
    ii.ipv6_addr.push_back(interface::ipv6_addr(name));
    ni.interfaces.push_back(ii);
  }
  ni.hostname = hostname();
  std::cout << "send " << hostname() << std::endl;
  anon_send(uplink, ni);
  spawn<hidden>([uplink](event_based_actor* self) -> behavior {
    self->send(self, atom("poll"));
    return {
      on(atom("poll")) >> [self] {
        self->delayed_send(self, std::chrono::seconds(1), atom("poll"));
        // TODO: collect RAM and CPU usage + send to uplink
      }
    };
  });
  return true;
}

} // namespace probe
} // namespace caf
