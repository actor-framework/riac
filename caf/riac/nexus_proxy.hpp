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

class nexus_proxy : public event_based_actor {
 protected:
  behavior make_behavior() override;

  std::vector<node_id> nodes_on_host(const std::string& hostname);

 private:
  riac::probe_data_map  m_data;
  std::list<node_id> m_visited_nodes;
};

} // namespace riac
} // namespace caf

#endif // CAF_RIAC_NEXUS_PROXY_HPP
