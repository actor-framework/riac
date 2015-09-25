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

#include <map>
#include <set>

#include "caf/typed_event_based_actor.hpp"

#include "caf/riac/message_types.hpp"

namespace caf {
namespace riac {

class nexus : public nexus_type::base {
public:
  nexus(bool silent);
  behavior_type make_behavior() override;

private:
  void broadcast();
  void add(listener_type hdl);

  bool silent_;
  std::map<actor_addr, node_id> probes_;
  probe_data_map data_;
  std::set<listener_type> listeners_;
};

} // namespace riac
} // namespace caf
