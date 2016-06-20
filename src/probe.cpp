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
  fwd_hook(actor_system& sys)
      : io::hook(sys),
        self_(sys, true),
        uplink_(unsafe_actor_handle_init),
        node_(sys.node()) {
    // nop
  }

  void register_at_nexus(nexus_type uplink) {
    CAF_ASSERT(self_.home_system().node() != invalid_node_id);
    uplink_ = std::move(uplink);
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
    if (! uplink_.unsafe())
      self_->send(uplink_, T{std::forward<Ts>(args)...});
  }

  void message_received_cb(const node_id&, const strong_actor_ptr& from,
                           const strong_actor_ptr& dest, message_id,
                           const message& msg) override {
    transmit<new_message>(node(from), node(dest), id(from), id(dest), msg);
  }

  void message_sent_cb(const strong_actor_ptr& from, const node_id&,
                       const strong_actor_ptr& dest, message_id,
                       const message& msg) override {
    // avoid endless recursion
    if (uplink_.unsafe() || dest == uplink_)
      return;
    transmit<new_message>(node(from), node(dest), id(from), id(dest), msg);
  }

  void message_forwarded_cb(const io::basp::header&,
                            const std::vector<char>*) override {
    // do nothing (yet)
  }

  void message_forwarding_failed_cb(const io::basp::header&,
                                    const std::vector<char>*) override {
    // do nothing (yet)
  }

  void message_sending_failed_cb(const strong_actor_ptr&,
                                 const strong_actor_ptr&, message_id,
                                 const message&) override {
    // do nothing (yet)
  }

  void actor_published_cb(const strong_actor_ptr& addr,
                          const std::set<std::string>&,
                          uint16_t port) override {
    transmit<new_actor_published>(node_, addr, port);
  }

  void new_remote_actor_cb(const strong_actor_ptr&) override {
    // do nothing (yet)
  }

  void new_connection_established_cb(const node_id& dest) override {
    transmit<new_route>(node_, dest, true);
  }

  void new_route_added_cb(const node_id&, const node_id& dest) override {
    transmit<new_route>(node_, dest, false);
  }

  void invalid_message_received_cb(const node_id&, const strong_actor_ptr&,
                                   actor_id, message_id,
                                   const message&) override {
    // do nothing (yet)
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
  auto is_fwd_hook = [](const io::hook_uptr& ptr) {
    return typeid(ptr.get()) == typeid(io::hook*);
  };
  auto& hooks = system_.middleman().hooks();
  auto e = hooks.end();
  auto i = std::find_if(hooks.begin(), e, is_fwd_hook);
  if (i == e)
    CAF_LOG_ERROR("unable to find fwd_hook!");
  else
    static_cast<fwd_hook*>(i->get())->register_at_nexus(uplink_);
}

void probe::stop() {
  // nop
}

void probe::init(actor_system_config& cfg) {
  CAF_LOG_TRACE("");
  add_message_types(cfg);
  nexus_port_ = cfg.nexus_port;
  nexus_host_ = std::move(cfg.nexus_host);
  cfg.add_hook_type<fwd_hook>();
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
