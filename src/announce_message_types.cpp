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

#include "caf/riac/announce_message_types.hpp"

#include <set>
#include <map>

#include "caf/maybe.hpp"
#include "caf/announce.hpp"

#include "caf/io/network/protocol.hpp"

#include "caf/riac/message_types.hpp"

namespace caf {
namespace riac {

void announce_message_types() {
  announce<cpu_info>("@cpu_info", &cpu_info::num_cores,
                     &cpu_info::mhz_per_core);
  announce<node_info>("@node_info", &node_info::source_node, &node_info::cpu,
                      &node_info::hostname, &node_info::os,
                      &node_info::interfaces);
  announce<node_disconnected>("@node_disconnected",
                              &node_disconnected::source_node);
  announce<ram_usage>("@ram_usage", &ram_usage::available, &ram_usage::in_use);
  announce<work_load>("@work_load", &work_load::cpu_load,
                      &work_load::num_actors, &work_load::num_processes);
  announce<new_route>("@new_route", &new_route::source_node, &new_route::dest,
                      &new_route::is_direct);
  announce<route_lost>("@route_lost", &route_lost::dest);
  announce<new_message>("@new_message", &new_message::source_node,
                        &new_message::dest_node, &new_message::source_actor,
                        &new_message::dest_actor, &new_message::msg);
  announce<add_listener>("@add_listener", &add_listener::listener);
  announce<add_typed_listener>("@add_typed_listener",
                               &add_typed_listener::listener);
  announce<maybe<ram_usage>>("@opt_ram_usage");
  announce<maybe<work_load>>("@opt_work_load");
  announce<std::set<node_id>>("@opt_node_id");
  announce<std::pair<actor_addr, uint16_t>>("@pub_info");
  announce<std::set<std::pair<actor_addr, uint16_t>>>("@pub_info_set");
  announce<std::set<actor_addr>>("@actor_addr_set");
  announce<new_actor_published>(
    "@new_actor_published", &new_actor_published::source_node,
    &new_actor_published::published_actor, &new_actor_published::port);
  announce<probe_data>("@probe_data", &probe_data::node, &probe_data::ram,
                       &probe_data::load, &probe_data::direct_routes,
                       &probe_data::published_actors,
                       &probe_data::known_actors);
  announce<probe_data_map>("@probe_data_map");
  announce<sink_type>("@sink_type");
  announce<nexus_type>("@nexus_type");
}

} // namespace riac
} // namespace caf
