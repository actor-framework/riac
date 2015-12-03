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

#include "caf/riac/add_message_types.hpp"

#include "caf/actor_system_config.hpp"

#include "caf/riac/message_types.hpp"

namespace caf {
namespace riac {

void add_message_types(actor_system_config& cfg) {
  cfg.add_message_type<cpu_info>("@cpu_info")
     .add_message_type<node_info>("@node_info")
     .add_message_type<node_disconnected>("@node_disconnected")
     .add_message_type<ram_usage>("@ram_usage")
     .add_message_type<work_load>("@work_load")
     .add_message_type<new_route>("@new_route")
     .add_message_type<route_lost>("@route_lost")
     .add_message_type<new_message>("@new_message")
     .add_message_type<add_listener>("@add_listener")
     .add_message_type<add_typed_listener>("@add_typed_listener")
     .add_message_type<maybe<ram_usage>>("@opt_ram_usage")
     .add_message_type<maybe<work_load>>("@opt_work_load")
     .add_message_type<std::set<node_id>>("@opt_node_id")
     .add_message_type<std::pair<actor_addr, uint16_t>>("@pub_info")
     .add_message_type<std::set<std::pair<actor_addr, uint16_t>>>("@pub_info_set")
     .add_message_type<std::set<actor_addr>>("@actor_addr_set")
     .add_message_type<new_actor_published>("@new_actor_published")
     .add_message_type<probe_data>("@probe_data")
     .add_message_type<probe_data_map>("@probe_data_map")
     .add_message_type<sink_type>("@sink_type")
     .add_message_type<nexus_type>("@nexus_type");
}

} // namespace riac
} // namespace caf
