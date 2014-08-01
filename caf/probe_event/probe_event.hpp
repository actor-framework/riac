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

#include <vector>
#include <cstdint>

#include "caf/node_id.hpp"

namespace caf {
namespace probe_event {


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

struct cpu_info {
  uint64_t  num_cores;
  uint64_t  mhz_per_core;
};

// send on connect from ActorProbe to ActorNexus
struct interface_info {
  std::string               hw_addr;
  std::string               ipv4_addr;
  std::vector<std::string>  ipv6_addr;
};

// send on connect from ActorProbe to ActorNexus
struct node_info {
  node_id                     id;
  std::vector<cpu_info>       cpu;
  std::string                 hostname;
  std::string                 os;
  std::vector<interface_info> interfaces;
};

// send from ActorProbe to ActorNexus whenever from learns a new direct or
// indirect route to another node
struct new_route {
  node_id   from;
  node_id   to;
  bool      is_direct;
};

struct new_message {
  node_id   source_node;
  node_id   dest_node;
  actor_id  source_actor;
  actor_id  dest_actor;
  message   msg;
};

inline void announce_types() {
}

} // namespace probe_event
} // namespace caf

