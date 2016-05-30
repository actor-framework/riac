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

#ifndef CAF_RIAC_NEXUS_PROXY_HPP
#define CAF_RIAC_NEXUS_PROXY_HPP

#include <vector>

#include "caf/all.hpp"
#include "caf/riac/all.hpp"

namespace caf {
namespace riac {

/// Used to query all known nodes from nexus
using list_nodes = atom_constant<atom("listNodes")>;

/// Used to query meta information about a particular node.
using get_node = atom_constant<atom("getNode")>;

/// Used to query all peers of a particular node.
using list_peers = atom_constant<atom("listPeers")>;

/// Used to query system load information on a particular node.
using get_sys_load = atom_constant<atom("getSysLoad")>;

/// Used to query RAM usage on a particular node.
using get_ram_usage = atom_constant<atom("getRam")>;

/// Used to query all known actors on a particular node.
using list_actors = atom_constant<atom("listActors")>;

/// Used to query a single actor on a particular node.
using get_actor = atom_constant<atom("getActor")>;

struct nexus_proxy_state {
  riac::probe_data_map data;
  std::list<node_id> visited_nodes;
};

using nexus_proxy_type =
  nexus_type::extend<
    reacts_to<probe_data_map>,
    replies_to<list_nodes>::with<std::vector<node_id>>,
    replies_to<list_nodes, std::string>::with<std::vector<node_id>>,
    replies_to<get_node, node_id>::with<node_info>,
    replies_to<list_peers, node_id>::with<std::vector<node_id>>,
    replies_to<get_sys_load, node_id>::with<work_load>,
    replies_to<get_ram_usage, node_id>::with<ram_usage>,
    replies_to<list_actors, node_id>::with<std::vector<strong_actor_ptr>>,
    replies_to<get_actor, node_id, actor_id>::with<strong_actor_ptr>
  >;

nexus_proxy_type::behavior_type
nexus_proxy(nexus_proxy_type::stateful_pointer<nexus_proxy_state> self);

} // namespace riac
} // namespace caf

#endif // CAF_RIAC_NEXUS_PROXY_HPP
