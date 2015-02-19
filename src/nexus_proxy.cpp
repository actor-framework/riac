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

std::vector<node_id> nexus_proxy::nodes_on_host(const std::string& hostname) {
  std::vector<node_id> accu;
  for (auto kvp : m_data) {
    if (kvp.second.node.hostname == hostname) {
      accu.push_back(kvp.first);
    }
  }
  return accu;
}

behavior nexus_proxy::make_behavior() {
  return {
    // nexus communication
    [=](const riac::new_message& ) {
      //aout(this) << "new message" << endl;
    },
    [=](const riac::new_route& route) {
      if (route.is_direct) {
        m_data[route.source_node].direct_routes.insert(route.dest);
      }
    },
    [=](riac::node_info& ni) {
      m_data[ni.source_node].node = std::move(ni);
    },
    [=](riac::work_load& wl) {
      m_data[wl.source_node].load = std::move(wl);
    },
    [=](riac::ram_usage& ru) {
      m_data[ru.source_node].ram = std::move(ru);
    },
    [=](const riac::new_actor_published& msg) {
      auto addr = msg.published_actor;
      auto nid = msg.source_node;
      if (addr == invalid_actor_addr) {
        return;
      }
      m_data[nid].known_actors.insert(addr);
      m_data[nid].published_actors.insert(std::make_pair(addr, msg.port));
    },
    on(atom("Nodes")) >> [=]() -> std::vector<node_id> {
      std::vector<node_id> result;
      result.reserve(m_data.size());
      for (auto& kvp : m_data) {
        result.push_back(kvp.first);
      }
      return result;
    },
    on(atom("HasNode"), arg_match) >> [=](const node_id& nid) -> message {
      auto i = m_data.find(nid);
      if (i != m_data.end()) {
        return make_message(atom("Yes"));
      }
      return make_message(atom("No"));
    },
    on(atom("OnHost"), arg_match) >> [=](const std::string& hostname)
                                                       -> std::vector<node_id> {
      return nodes_on_host(hostname);
    },
    on(atom("Routes"), arg_match) >> [=](const node_id& ni) -> std::set<node_id> {
      auto kvp = m_data.find(ni);
      if(kvp != m_data.end()) {
        probe_data pd = kvp->second;
        return pd.direct_routes;
      }
      return std::set<node_id>{};
    },
    on(atom("NodeInfo"), arg_match) >> [=](const node_id& nid) -> message {
      auto i = m_data.find(nid);
      if (i == m_data.end()) {
        return make_message(atom("NoNodeInfo"));
      }
      return make_message(i->second.node);
    },
    on(atom("WorkLoad"), arg_match) >> [=](const node_id& nid) -> message {
      auto i = m_data.find(nid);
      if (i == m_data.end() || !i->second.load) {
        return make_message(atom("NoWorkLoad"));
      }
      return make_message(*(i->second.load));
    },
    on(atom("RamUsage"), arg_match) >> [=](const node_id& nid) -> message {
      auto i = m_data.find(nid);
      if (i == m_data.end() || !i->second.ram) {
        return make_message(atom("NoRamUsage"));
      }
      return make_message(*(i->second.ram));
    },
    on(atom("ListActors"), arg_match) >> [=](const node_id& nid) -> std::string {
      std::ostringstream oss;
      auto& known_actors = m_data[nid].known_actors;
      for (auto& addr : known_actors) {
        oss << addr.id() << "\n";
      }
      return oss.str();
    },
    on(atom("GetActor"), arg_match) >> [=](const node_id& nid, uint32_t aid) -> actor {
      auto& known_actors = m_data[nid].known_actors;
      auto last = known_actors.end();
      auto pred = [aid](const actor_addr& addr) {
        return addr.id() == aid;
      };
      auto i = std::find_if(known_actors.begin(), last, pred);
      if (i != last) {
        return actor_cast<actor>(*i);
      }
      return invalid_actor;
    },
    on(atom("Init"), arg_match) >> [=](const riac::nexus_type& nexus) {
      auto hdl = make_response_promise();
      send(nexus, riac::add_listener{this});
      become(
        keep_behavior,
        [=](riac::probe_data_map& init_state) {
          m_data.swap(init_state);
          hdl.deliver(make_message(atom("InitDone")));
          unbecome();
        }
      );
    },
    [=](const down_msg& ) {
      // nop
    },
    [=](const riac::node_disconnected& nd) {
      m_data.erase(nd.source_node);
    },
    others() >> [=] {
      aout(this) << "Received from sender: "
                 << to_string(current_sender())
                 << std::endl << "an unexpected message. "
                 << to_string(current_message())
                 << std::endl << std::endl;
    }
  };
}

} // namespace riac
} // namespace caf
