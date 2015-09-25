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

#include "caf/riac/nexus.hpp"

#include <iostream>

#include "caf/actor_ostream.hpp"

using std::cerr;
using std::endl;

#define CHECK_SOURCE(TypeName, VarName)                                        \
  if (VarName.source_node == caf::invalid_node_id) {                           \
    cerr << #TypeName << " received with invalid source node" << endl;         \
    return;                                                                    \
  } else {                                                                     \
    if (! silent_)                                                             \
      aout(this) << "received " << #TypeName << endl;                          \
  }                                                                            \
  static_cast<void>(0)

#define HANDLE_UPDATE(TypeName, FieldName)                                     \
  [=](const TypeName& FieldName) {                                             \
    if (FieldName.source_node == caf::invalid_node_id) {                       \
      cerr << #TypeName << " received with invalid source node" << endl;       \
      return;                                                                  \
    }                                                                          \
    if (! silent_)                                                             \
      aout(this) << "received " << #TypeName << endl;                          \
    data_[FieldName.source_node].FieldName = FieldName;                        \
    broadcast();                                                               \
  }

namespace {

std::string format_down_msg(const std::string& type, const caf::down_msg& dm) {
  std::stringstream ds;
  ds << type << " "
     << to_string(dm.source) << " exited with reason "
     << dm.reason;
  return ds.str();
}

} // namespace <anonymous>

namespace caf {
namespace riac {

nexus::nexus(bool silent) : silent_(silent) {
  // nop
}

void nexus::add(listener_type hdl) {
  if (listeners_.insert(hdl).second) {
    if (! silent_)
      aout(this) << "new listener: "
                 << to_string(actor_cast<actor>(hdl)) << endl;
    monitor(hdl);
    send(hdl, data_);
  }
}

void nexus::broadcast() {
  for (auto& l : listeners_)
    send(actor_cast<actor>(l), current_message());
}

nexus::behavior_type nexus::make_behavior() {
  return {
    [=](const node_info& ni, const actor& observer) {
      if (ni.source_node == caf::invalid_node_id) {
        cerr << "node_info received with invalid source node" << endl;
        return;
      }
      if (! silent_)
        aout(this) << "received node_info: "
                   << to_string(current_message()) << endl;
      data_[ni.source_node].node = ni;
      auto& ls = current_sender();
      probes_[ls] = ls.node();
      monitor(ls);
      broadcast();
      if (observer != invalid_actor)
        send(observer, ok_atom::value, nexus_type{this});
    },
    HANDLE_UPDATE(ram_usage, ram),
    HANDLE_UPDATE(work_load, load),
    [=](const new_actor_published& msg) {
      CHECK_SOURCE(actor_published, msg);
      auto addr = msg.published_actor;
      auto nid = msg.source_node;
      if (addr == invalid_actor_addr) {
        cerr << "received actor_published "
             << "with invalid actor address"
             << endl;
        return;
      }
      if (data_[nid].known_actors.insert(addr).second) {
        monitor(addr);
      }
      data_[nid].published_actors.insert(std::make_pair(addr, msg.port));
      broadcast();
    },
    [=](const new_route& route) {
      CHECK_SOURCE(new_route, route);
      if (route.is_direct
          && data_[route.source_node].direct_routes.insert(route.dest).second) {
        broadcast();
      }
    },
    [=](const route_lost& route) {
      CHECK_SOURCE(route_lost, route);
      if (data_[route.source_node].direct_routes.erase(route.dest) > 0) {
        if (! silent_)
          aout(this) << "new route" << endl;
        broadcast();
      }
    },
    [=](const new_message& msg) {
      // TODO: reduce message size by avoiding the complete msg
      CHECK_SOURCE(new_message, msg);
      if (! silent_)
        aout(this) << "new message: " << to_string(msg.msg) << endl;
      broadcast();
    },
    [=](const add_listener& req) {
      add(actor_cast<listener_type>(req.listener));
    },
    [=](const add_typed_listener& req) {
      add(req.listener);
    },
    [=](const down_msg& dm) {
      if (listeners_.erase(actor_cast<listener_type>(dm.source)) > 0) {
        if (! silent_)
          aout(this) << format_down_msg("listener", dm) << endl;
        return;
      }
      auto probe_addr = probes_.find(dm.source);
      if (probe_addr != probes_.end()) {
        if (! silent_)
          aout(this) << format_down_msg("probe", dm) << endl;
        node_disconnected nd{probe_addr->second};
        send(this, nd);
        auto i = data_.find(probe_addr->second);
        if (i != data_.end()
            && i->second.known_actors.erase(probe_addr->first) > 0) {
          return;
        }
      }
    },
    [=](const node_disconnected& nd) {
      data_.erase(nd.source_node);
      broadcast();
    }
  };
}

} // namespace riac
} // namespace caf
