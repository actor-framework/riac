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

#include "caf/riac/nexus_proxy.hpp"

namespace caf {
namespace riac {

nexus_proxy_type::behavior_type
nexus_proxy(nexus_proxy_type::stateful_pointer<nexus_proxy_state> self) {
  return {
    // from sink_type
    [=](node_info& ni, const actor& observer) {
      self->state.data[ni.source_node].node = std::move(ni);
      if (observer != invalid_actor)
        self->send(observer, ok_atom::value, nexus_type{self});
    },
    [=](ram_usage& ru) {
      self->state.data[ru.source_node].ram = std::move(ru);
    },
    [=](work_load& wl) {
      self->state.data[wl.source_node].load = std::move(wl);
    },
    [=](const new_route& route) {
      if (route.is_direct)
        self->state.data[route.source_node].direct_routes.insert(route.dest);
    },
    [=](const route_lost& route) {
      self->state.data[route.source_node].direct_routes.erase(route.dest);
    },
    [=](const new_message&) {
      //aout(this) << "new message" << endl;
    },
    [=](const new_actor_published& msg) {
      auto addr = msg.published_actor;
      auto nid = msg.source_node;
      if (addr == invalid_actor_addr) {
        return;
      }
      self->state.data[nid].known_actors.insert(addr);
      self->state.data[nid].published_actors.insert(std::make_pair(addr, msg.port));
    },
    [=](const node_disconnected& nd) {
      self->state.data.erase(nd.source_node);
    },
    // from nexus_type
    [=](const add_listener&) {
      // TODO
    },
    [=](const add_typed_listener&) {
      // TODO
    },
    // from nexus_proxy_type
    [=](probe_data_map& new_data) {
      self->state.data = std::move(new_data);
    },
    [=](list_nodes) -> std::vector<node_id> {
      std::vector<node_id> result;
      result.reserve(self->state.data.size());
      for (auto& kvp : self->state.data)
        result.push_back(kvp.first);
      return result;
    },
    [=](list_nodes, const std::string& hostname) -> std::vector<node_id> {
      std::vector<node_id> result;
      for (auto& kvp : self->state.data)
        if (kvp.second.node.hostname == hostname)
          result.push_back(kvp.first);
      return result;
    },
    [=](get_node, const node_id& nid) -> maybe<node_info> {
      auto i = self->state.data.find(nid);
      if (i == self->state.data.end())
        return sec::no_such_riac_node;
      return i->second.node;
    },
    [=](list_peers, const node_id& ni) -> std::vector<node_id> {
      std::vector<node_id> result;
      auto kvp = self->state.data.find(ni);
      if(kvp != self->state.data.end()) {
        auto& direct_routes = kvp->second.direct_routes;
        result.insert(result.end(), direct_routes.begin(), direct_routes.end());
      }
      return result;
    },
    [=](get_sys_load, const node_id& nid) -> maybe<work_load> {
      auto i = self->state.data.find(nid);
      if (i == self->state.data.end() || ! i->second.load) {
        return sec::no_such_riac_node;
      }
      return *(i->second.load);
    },
    [=](get_ram_usage, const node_id& nid) -> maybe<ram_usage> {
      auto i = self->state.data.find(nid);
      if (i == self->state.data.end() || ! i->second.ram)
        return sec::no_such_riac_node;
      return *(i->second.ram);
    },
    [=](list_actors, const node_id& nid) -> std::vector<actor_addr> {
      std::vector<actor_addr> result;
      for (auto& addr : self->state.data[nid].known_actors)
        result.push_back(addr);
      return result;
    },
    [=](get_actor, const node_id& nid, actor_id aid) -> actor_addr {
      auto& known_actors = self->state.data[nid].known_actors;
      auto last = known_actors.end();
      auto pred = [aid](const actor_addr& addr) {
        return addr.id() == aid;
      };
      auto i = std::find_if(known_actors.begin(), last, pred);
      if (i != last)
        return *i;
      return invalid_actor_addr;
    },
    [=](const down_msg& ) {
      // nop
    },
  };
}

} // namespace riac
} // namespace caf
