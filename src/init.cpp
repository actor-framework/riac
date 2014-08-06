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

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "caf/probe_event/all.hpp"

namespace caf {
namespace probe {

namespace {

class fwd_hook : public io::hook {
 public:
  fwd_hook(probe_event::nexus_type uplink) : self(true), m_uplink(uplink) {
    // nop
  }
  template<class T, class... Ts>
  void transmit(Ts&&... args) {
    self->send(m_uplink, T{std::forward<Ts>(args)...});
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

  void new_connection_established_cb(const node_id& node) override {
    transmit<probe_event::new_route>(node, true);
    call_next<new_connection_established>(node);
  }

  void new_route_added_cb(const node_id& via, const node_id& node) override {
    transmit<probe_event::new_route>(node, false);
    call_next<new_route_added>(via, node);
  }

  void invalid_message_received_cb(const node_id& source,
                                   const actor_addr& sender,
                                   actor_id invalid_dest, message_id mid,
                                   const message& msg) override {
    // do nothing (yet)
    call_next<invalid_message_received>(source, sender, invalid_dest, mid, msg);
  }

 private:
  scoped_actor self;
  probe_event::nexus_type m_uplink;
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
const char host_arg[] = "--caf_probe_host=";
const char port_arg[] = "--caf_probe_port=";

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
  return true;
}

} // namespace probe
} // namespace caf
