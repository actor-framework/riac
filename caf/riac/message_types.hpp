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

#ifndef CAF_RIAC_MESSAGE_TYPES_HPP
#define CAF_RIAC_MESSAGE_TYPES_HPP

#include <string>
#include <vector>
#include <cstdint>

#include "caf/maybe.hpp"
#include "caf/actor.hpp"
#include "caf/node_id.hpp"
#include "caf/typed_actor.hpp"

#include "caf/io/network/interfaces.hpp"

namespace caf {
namespace riac {

struct cpu_info {
  node_id source_node;
  uint64_t num_cores;
  uint64_t mhz_per_core;
};

inline bool operator==(const cpu_info& lhs, const cpu_info& rhs) {
  return lhs.source_node == rhs.source_node
         && lhs.num_cores == rhs.num_cores
         && lhs.mhz_per_core == rhs.mhz_per_core;
}

template <class T>
void serialize(T& in_or_out, cpu_info& x, const unsigned int) {
  in_or_out & x.source_node;
  in_or_out & x.num_cores;
  in_or_out & x.mhz_per_core;
}

// send on connect from ActorProbe to ActorNexus
struct node_info {
  node_id source_node;
  std::vector<cpu_info> cpu;
  std::string hostname;
  std::string os;
  io::network::interfaces_map interfaces;
};

inline bool operator==(const node_info& lhs, const node_info& rhs) {
  return lhs.source_node == rhs.source_node
         && lhs.cpu == rhs.cpu
         && lhs.hostname == rhs.hostname
         && lhs.os == rhs.os
         && lhs.interfaces == rhs.interfaces;
}

template <class T>
void serialize(T& in_or_out, node_info& x, const unsigned int) {
  in_or_out & x.source_node;
  in_or_out & x.cpu;
  in_or_out & x.hostname;
  in_or_out & x.os;
  in_or_out & x.interfaces;
}

struct node_disconnected {
  node_id source_node;
};

inline bool operator==(const node_disconnected& lhs,
                       const node_disconnected& rhs) {
  return lhs.source_node == rhs.source_node;
}

template <class T>
void serialize(T& in_or_out, node_disconnected& x, const unsigned int) {
  in_or_out & x.source_node;
}

// send periodically from ActorProbe to ActorNexus
struct ram_usage {
  node_id source_node;
  uint64_t in_use;
  uint64_t available;
};

inline bool operator==(const ram_usage& lhs, const ram_usage& rhs) {
  return lhs.source_node == rhs.source_node
         && lhs.in_use == rhs.in_use
         && lhs.available == rhs.available;
}

template <class T>
void serialize(T& in_or_out, ram_usage& x, const unsigned int) {
  in_or_out & x.source_node;
  in_or_out & x.in_use;
  in_or_out & x.available;
}

// send periodically from ActorProbe to ActorNexus
struct work_load {
  node_id source_node;
  uint8_t cpu_load; // in percent, i.e., 0-100
  uint64_t num_processes;
  uint64_t num_actors;
};

inline bool operator==(const work_load& lhs, const work_load& rhs) {
  return lhs.source_node == rhs.source_node
         && lhs.cpu_load == rhs.cpu_load
         && lhs.num_processes == rhs.num_processes
         && lhs.num_actors == rhs.num_actors;
}

template <class T>
void serialize(T& in_or_out, work_load& x, const unsigned int) {
  in_or_out & x.source_node;
  in_or_out & x.cpu_load;
  in_or_out & x.num_processes;
  in_or_out & x.num_actors;
}

// send from ActorProbe to ActorNexus whenever from learns a new direct or
// indirect route to another node
struct new_route {
  node_id source_node;
  node_id dest;
  bool is_direct;
};

inline bool operator==(const new_route& lhs, const new_route& rhs) {
  return lhs.dest == rhs.dest && lhs.is_direct == rhs.is_direct;
}

template <class T>
void serialize(T& in_or_out, new_route& x, const unsigned int) {
  in_or_out & x.source_node;
  in_or_out & x.dest;
  in_or_out & x.is_direct;
}

struct route_lost {
  node_id source_node;
  node_id dest;
};

inline bool operator==(const route_lost& lhs, const route_lost& rhs) {
  return lhs.dest == rhs.dest;
}

template <class T>
void serialize(T& in_or_out, route_lost& x, const unsigned int) {
  in_or_out & x.source_node;
  in_or_out & x.dest;
}

struct new_message {
  node_id source_node;
  node_id dest_node;
  actor_id source_actor;
  actor_id dest_actor;
  maybe<message> msg;
};

inline bool operator==(const new_message& lhs, const new_message& rhs) {
  return lhs.source_node == rhs.source_node
         && lhs.dest_node == rhs.dest_node
         && lhs.source_actor == rhs.source_actor
         && lhs.dest_actor == rhs.dest_actor
         && lhs.msg == rhs.msg;
}

template <class T>
void serialize(T& in_or_out, new_message& x, const unsigned int) {
  in_or_out & x.source_node;
  in_or_out & x.dest_node;
  in_or_out & x.source_actor;
  in_or_out & x.dest_actor;
  in_or_out & x.msg;
}

struct new_actor_published {
  node_id source_node;
  actor_addr published_actor;
  uint16_t port;
};

inline bool operator==(const new_actor_published& lhs,
                       const new_actor_published& rhs) {
  return lhs.source_node == rhs.source_node
         && lhs.published_actor == rhs.published_actor
         && lhs.port == rhs.port;
}

template <class T>
void serialize(T& in_or_out, new_actor_published& x, const unsigned int) {
  in_or_out & x.source_node;
  in_or_out & x.published_actor;
  in_or_out & x.port;
}

/// Convenience structure to store data collected from probes.
struct probe_data {
  node_info node;
  maybe<ram_usage> ram;
  maybe<work_load> load;
  std::set<node_id> direct_routes;
  std::set<std::pair<actor_addr, uint16_t>> published_actors;
  std::set<actor_addr> known_actors;
};

inline bool operator==(const probe_data& lhs, const probe_data& rhs) {
  return lhs.node == rhs.node
         && lhs.ram == rhs.ram
         && lhs.load == rhs.load
         && lhs.direct_routes == rhs.direct_routes;
}

template <class T>
void serialize(T& in_or_out, probe_data& x, const unsigned int) {
  in_or_out & x.node;
  in_or_out & x.ram;
  in_or_out & x.load;
  in_or_out & x.direct_routes;
  in_or_out & x.published_actors;
  in_or_out & x.known_actors;
}

using probe_data_map = std::map<node_id, probe_data>;

/// An event sink consuming messages from the probes.
using sink_type = typed_actor<reacts_to<node_info, actor>,
                              reacts_to<ram_usage>,
                              reacts_to<work_load>,
                              reacts_to<new_route>,
                              reacts_to<route_lost>,
                              reacts_to<new_message>,
                              reacts_to<new_actor_published>,
                              reacts_to<node_disconnected>>;

using listener_type = sink_type::extend<reacts_to<probe_data_map>>;

struct add_listener {
  actor listener;
};

inline bool operator==(const add_listener& lhs, const add_listener& rhs) {
  return lhs.listener == rhs.listener;
}

template <class T>
void serialize(T& in_or_out, add_listener& x, const unsigned int) {
  in_or_out & x.listener;
}

struct add_typed_listener {
  listener_type listener;
};

inline bool operator==(const add_typed_listener& lhs,
                       const add_typed_listener& rhs) {
  return lhs.listener == rhs.listener;
}

template <class T>
void serialize(T& in_or_out, add_typed_listener& x, const unsigned int) {
  in_or_out & x.listener;
}

/// The expected type of the nexus.
using nexus_type = sink_type::extend<reacts_to<add_listener>,
                                     reacts_to<add_typed_listener>>;

} // namespace riac
} // namespace caf

#endif // CAF_RIAC_MESSAGE_TYPES_HPP
