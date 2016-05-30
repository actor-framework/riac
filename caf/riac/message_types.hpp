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

#include "caf/actor.hpp"
#include "caf/node_id.hpp"
#include "caf/optional.hpp"
#include "caf/typed_actor.hpp"

#include "caf/io/network/interfaces.hpp"

namespace caf {
namespace riac {

struct cpu_info {
  node_id source_node;
  uint64_t num_cores;
  uint64_t mhz_per_core;
};

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

inline std::string to_string(const node_info& x) {
  return "node_info" + deep_to_string(std::forward_as_tuple(x.source_node,
                                                            x.cpu,
                                                            x.hostname,
                                                            x.os,
                                                            x.interfaces));
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

inline std::string to_string(const node_disconnected& x) {
  return "node_disconnected" + deep_to_string_as_tuple(x.source_node);
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
  optional<message> msg;
};

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
  strong_actor_ptr published_actor;
  uint16_t port;
};

template <class T>
void serialize(T& in_or_out, new_actor_published& x, const unsigned int) {
  in_or_out & x.source_node;
  in_or_out & x.published_actor;
  in_or_out & x.port;
}

/// Convenience structure to store data collected from probes.
struct probe_data {
  node_info node;
  optional<ram_usage> ram;
  optional<work_load> load;
  std::set<node_id> direct_routes;
  std::set<std::pair<strong_actor_ptr, uint16_t>> published_actors;
  std::set<strong_actor_ptr> known_actors;
};

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
using sink_type = typed_actor<reacts_to<node_info>,
                              reacts_to<ram_usage>,
                              reacts_to<work_load>,
                              reacts_to<new_route>,
                              reacts_to<route_lost>,
                              reacts_to<new_message>,
                              reacts_to<new_actor_published>,
                              reacts_to<node_disconnected>>;

using listener_type = sink_type::extend<reacts_to<probe_data_map>>;

/// The expected type of the nexus.
using nexus_type = sink_type::extend<reacts_to<add_atom, actor>,
                                     reacts_to<add_atom, listener_type>>;

} // namespace riac
} // namespace caf

#endif // CAF_RIAC_MESSAGE_TYPES_HPP
