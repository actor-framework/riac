/******************************************************************************
 *                       ____    _    _____                                   *
 *                      / ___|  / \  |  ___|    C++                           *
 *                     | |     / _ \ | |_       Actor                         *
 *                     | |___ / ___ \|  _|      Framework                     *
 *                      \____/_/   \_|_|                                      *
 *                                                                            *
 * Copyright (C) 2011 - 2015                                                  *
 * Dominik Charousset <dominik.charousset (at) haw-hamburg.de>                *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 *                                                                            *
 * If you did not receive a copy of the license files, see                    *
 * http://opensource.org/licenses/BSD-3-Clause and                            *
 * http://www.boost.org/LICENSE_1_0.txt.                                      *
 ******************************************************************************/

#include "caf/riac/probe.hpp"

#ifdef CAF_WINDOWS
#include <Winsock2.h>
#else
#include <unistd.h>
#endif

#include <cstring>
#include <fstream>
#include <iostream>

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "caf/riac/nexus.hpp"
#include "caf/riac/add_message_types.hpp"

#include "caf/io/network/interfaces.hpp"

namespace caf {
namespace riac {

namespace {

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

class fwd_hook : public io::hook {
public:
  fwd_hook(actor_system& sys, nexus_type uplink)
      : self_(sys, true),
        uplink_(uplink),
        node_(sys.node()) {
    // nop
  }

  void register_at_nexus() {
    CAF_ASSERT(self_.home_system().node() != invalid_node_id);
    node_info ni;
    ni.source_node = self_.home_system().node();
    ni.interfaces = io::network::interfaces::list_all();
    ni.hostname = hostname();
    self_->request(uplink_, infinite, std::move(ni)).receive(
      [] {
        // nop
      }
    );
  }

  node_id node(const strong_actor_ptr& x) {
    return x ? invalid_node_id : x->node();
  }

  actor_id id(const strong_actor_ptr& x) {
    return x ? invalid_actor_id : x->id();
  }

  template<class T, class... Ts>
  void transmit(Ts&&... args) {
    self_->send(uplink_, T{std::forward<Ts>(args)...});
  }

  void message_received_cb(const node_id& src, const strong_actor_ptr& from,
                           const strong_actor_ptr& dest, message_id mid,
                           const message& msg) override {
    transmit<new_message>(node(from), node(dest), id(from), id(dest), msg);
    call_next<message_received>(src, from, dest, mid, msg);
  }

  void message_sent_cb(const strong_actor_ptr& from, const node_id& hop,
                       const strong_actor_ptr& dest, message_id mid,
                       const message& msg) override {
    // avoid endless recursion
    if (dest == uplink_)
      return;
    transmit<new_message>(node(from), node(dest), id(from), id(dest), msg);
    call_next<message_sent>(from, hop, dest, mid, msg);
  }

  void message_forwarded_cb(const io::basp::header& hdr,
                            const std::vector<char>* payload) override {
    // do nothing (yet)
    call_next<message_forwarded>(hdr, payload);
  }

  void message_forwarding_failed_cb(const io::basp::header& hdr,
                                    const std::vector<char>* payload) override {
    // do nothing (yet)
    call_next<message_forwarding_failed>(hdr, payload);
  }

  void message_sending_failed_cb(const strong_actor_ptr& from,
                                 const strong_actor_ptr& dest, message_id mid,
                                 const message& payload) override {
    // do nothing (yet)
    call_next<message_sending_failed>(from, dest, mid, payload);
  }

  void actor_published_cb(const strong_actor_ptr& addr,
                          const std::set<std::string>& ifs,
                          uint16_t port) override {
    transmit<new_actor_published>(node_, addr, port);
    call_next<actor_published>(addr, ifs, port);
  }

  void new_remote_actor_cb(const strong_actor_ptr& addr) override {
    // do nothing (yet)
    call_next<new_remote_actor>(addr);
  }

  void new_connection_established_cb(const node_id& dest) override {
    transmit<new_route>(node_, dest, true);
    call_next<new_connection_established>(dest);
  }

  void new_route_added_cb(const node_id& via, const node_id& dest) override {
    transmit<new_route>(node_, dest, false);
    call_next<new_route_added>(via, dest);
  }

  void invalid_message_received_cb(const node_id& source,
                                   const strong_actor_ptr& sender,
                                   actor_id invalid_dest, message_id mid,
                                   const message& msg) override {
    // do nothing (yet)
    call_next<invalid_message_received>(source, sender, invalid_dest, mid, msg);
  }

private:
  scoped_actor self_;
  nexus_type uplink_;
  node_id node_;
};

} // namespace <anonymous>

probe::probe(actor_system& sys)
    : system_(sys),
      uplink_(unsafe_actor_handle_init) {
  // nop
}

void probe::start() {
  CAF_LOG_TRACE("");
  try {
    uplink_ = system_.middleman().typed_remote_actor<nexus_type>(nexus_host_,
                                                                 nexus_port_);
  }
  catch (std::exception& e) {
    CAF_LOG_ERROR("could not connect to Nexus:"
                  << CAF_ARG(nexus_host_) << CAF_ARG(nexus_port_)
                  << CAF_ARG(e.what()));
    return;
  }
  auto ptr = system_.middleman().add_hook<fwd_hook>(uplink_);
  ptr->register_at_nexus();
}

void probe::stop() {
  // nop
}

void probe::init(actor_system_config& cfg) {
  CAF_LOG_TRACE("");
  add_message_types(cfg);
  nexus_port_ = cfg.nexus_port;
  nexus_host_ = std::move(cfg.nexus_host);
}

actor_system::module::id_t probe::id() const {
  return actor_system::module::riac_probe;
}

bool probe::connected() {
  return ! uplink_.unsafe();
}

void* probe::subtype_ptr() {
  return this;
}

actor_system::module* probe::make(actor_system& sys, detail::type_list<>) {
  return new probe{sys};
}

} // namespace riac
} // namespace caf
