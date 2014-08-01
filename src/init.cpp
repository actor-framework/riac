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

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "caf/probe_event/all.hpp"

namespace caf {
namespace probe {

class fwd_hook : public io::hook {
 public:
  fwd_hook(actor nexus) : m_uplink(nexus) {
    // nop
  }
  template<class T, class... Ts>
  void transmit(Ts&&... args) {
    anon_send(m_uplink, T{std::forward<Ts>(args)...});
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
    transmit<probe_event::new_message>(from.node(), dest.node(), from.id(),
                                       dest.id(), msg);
    call_next<message_sent>(from, hop, dest, mid, msg);
  }

  void message_forwarded_cb(const node_id& from, const node_id& dest,
                            std::vector<char>* payload) override {
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
                                    std::vector<char>* payload) override {
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
    transmit<probe_event::new_route>(detail::singletons::get_node_id(),
                                     node, true);
    call_next<new_connection_established>(node);
  }

  void new_route_added_cb(const node_id& via, const node_id& node) override {
    transmit<probe_event::new_route>(detail::singletons::get_node_id(),
                                     node, false);
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
  actor m_uplink;
};

behavior nexus_broker(io::broker* self, io::connection_handle conn) {
  return {
    // TODO: implement me
  };
}

void init(int argc, char** argv) {
  probe_event::announce_types();
  auto uplink = io::spawn_io_client(nexus_broker, "localhost", 4242);
  io::middleman::instance()->add_hook<fwd_hook>(uplink);
}

} // namespace probe
} // namespace caf

