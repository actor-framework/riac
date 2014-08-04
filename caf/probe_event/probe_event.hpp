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

#include <string>
#include <vector>
#include <cstdint>

#include "caf/actor.hpp"
#include "caf/node_id.hpp"
#include "caf/typed_actor.hpp"

namespace caf {
namespace probe_event {

struct cpu_info {
  uint64_t  num_cores;
  uint64_t  mhz_per_core;
};

inline bool operator==(const cpu_info& lhs, const cpu_info& rhs) {
  return lhs.num_cores == rhs.num_cores && lhs.mhz_per_core == rhs.mhz_per_core;
}

// send on connect from ActorProbe to ActorNexus
struct interface_info {
  std::string               hw_addr;
  std::string               ipv4_addr;
  std::vector<std::string>  ipv6_addr;
};

inline bool operator==(const interface_info& lhs, const interface_info& rhs) {
  return lhs.hw_addr == rhs.hw_addr
         && lhs.ipv4_addr == rhs.ipv4_addr
         && lhs.ipv6_addr == rhs.ipv6_addr;
}

// send on connect from ActorProbe to ActorNexus
struct node_info {
  node_id                     id;
  std::vector<cpu_info>       cpu;
  std::string                 hostname;
  std::string                 os;
  std::vector<interface_info> interfaces;
};

inline bool operator==(const node_info& lhs, const node_info& rhs) {
  return lhs.id == rhs.id
         && lhs.cpu == rhs.cpu
         && lhs.hostname == rhs.hostname
         && lhs.os == rhs.os
         && lhs.interfaces == rhs.interfaces;
}

// send periodically from ActorProbe to ActorNexus
struct ram_usage {
  uint64_t in_use;
  uint64_t available;
};

// send periodically from ActorProbe to ActorNexus
struct work_load {
  uint8_t   cpu_load;      // in percent, i.e., 0-100
  uint64_t  num_processes;
  uint64_t  num_actors;
};

// send from ActorProbe to ActorNexus whenever from learns a new direct or
// indirect route to another node
struct new_route {
  node_id to;
  bool    is_direct;
};

inline bool operator==(const new_route& lhs, const new_route& rhs) {
  return lhs.to == rhs.to && lhs.is_direct == rhs.is_direct;
}

struct route_lost {
  node_id to;
};

inline bool operator==(const route_lost& lhs, const route_lost& rhs) {
  return lhs.to == rhs.to;
}

struct new_message {
  node_id   source_node;
  node_id   dest_node;
  actor_id  source_actor;
  actor_id  dest_actor;
  message   msg;
};

inline bool operator==(const new_message& lhs, const new_message& rhs) {
  return lhs.source_node == rhs.source_node
         && lhs.dest_node == rhs.dest_node
         && lhs.source_actor == rhs.source_actor
         && lhs.dest_actor == rhs.dest_actor
         && lhs.msg == rhs.msg;
}

/**
 * An event sink consuming messages from the probes.
 */
using sink = typed_actor<reacts_to<node_info>,
                         reacts_to<ram_usage>,
                         reacts_to<work_load>,
                         reacts_to<new_route>,
                         reacts_to<route_lost>,
                         reacts_to<new_message>>;

struct add_listener {
  actor listener;
};

inline bool operator==(const add_listener& lhs, const add_listener& rhs) {
  return lhs.listener == rhs.listener;
}

struct add_typed_listener {
  sink listener;
};

inline bool operator==(const add_typed_listener& lhs,
                       const add_typed_listener& rhs) {
  return lhs.listener == rhs.listener;
}

/**
 * The expected type of the nexus.
 */
using nexus_type = sink::extend<
                     reacts_to<add_listener>,
                     reacts_to<add_typed_listener>
                   >::type;

/**
 * Announces all types used either probes or nexi to the type system.
 */
inline void announce_types() {
  announce<cpu_info>(&cpu_info::num_cores, &cpu_info::mhz_per_core);
  announce<interface_info>(&interface_info::hw_addr, &interface_info::ipv4_addr,
                           &interface_info::ipv6_addr);
  announce<node_info>(&node_info::id, &node_info::cpu, &node_info::hostname,
                      &node_info::os, &node_info::interfaces);
  announce<ram_usage>(&ram_usage::available, &ram_usage::in_use);
  announce<work_load>(&work_load::cpu_load, &work_load::num_actors,
                      &work_load::num_processes);
  announce<new_route>(&new_route::to, &new_route::is_direct);
  announce<route_lost>(&route_lost::to);
  announce<new_message>(&new_message::source_node, &new_message::dest_node,
                        &new_message::source_actor, &new_message::dest_actor,
                        &new_message::msg);
  announce<add_listener>(&add_listener::listener);
  announce<add_typed_listener>(&add_typed_listener::listener);
}

} // namespace probe_event
} // namespace caf

