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

#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iostream>

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "caf/probe/interfaces.hpp"
#include "caf/probe_event/all.hpp"

#include "caf/detail/singletons.hpp"
#include "cppa/opt.hpp"

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
    transmit<probe_event::actor_published>(m_node, addr, port);
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

// SUSv2 guarantees that "host names are limited to 255 bytes"
static constexpr size_t max_hostname_size = 256;

std::string hostname() {
  char buffer[max_hostname_size];
  gethostname(buffer, max_hostname_size);
  // make sure string is null terminated, because the POSIX standard does not
  // guarantee whether the returned buffer includes a terminating null byte in
  // case of a truncation
  buffer[max_hostname_size - 1] = '\0';
  return buffer;
}

} // namespace <anonymous>

bool init(const std::string& host, uint16_t port) {
  probe_event::announce_types();
  auto uplink = io::typed_remote_actor<probe_event::nexus_type>(host, port);
  io::middleman::instance()->add_hook<fwd_hook>(uplink);
  probe_event::node_info ni;
  ni.source_node = detail::singletons::get_node_id();
  ni.interfaces = interfaces();
  ni.hostname = hostname();
  anon_send(uplink, ni);
  /*
  // TODO: collect RAM and CPU usage + send to uplink
  spawn<hidden>([uplink](event_based_actor* self) -> behavior {
    self->send(self, atom("poll"));
    return {
      on(atom("poll")) >> [self] {
        self->delayed_send(self, std::chrono::seconds(1), atom("poll"));
      }
    };
  });
  */
  return true;
}

bool init(int argc, char** argv) {
  std::string host;
  uint16_t port = 0;
  options_description desc;
  bool args_valid = match_stream<std::string>(argv + 1, argv + argc) (
    on_opt1('H', "caf-nexus-host", &desc, "set nexus host") >> rd_arg(host),
    on_opt1('p', "caf-nexus-port", &desc, "set nexus port") >> rd_arg(port),
    on_opt0('h', "help", &desc, "print help") >> []() { return false; }
  );
  if (!args_valid || port == 0 || host.empty()) {
    print_desc(&desc, std::cerr)();
    return false;
  }
  return init(host, port);
}

} // namespace probe
} // namespace caf
