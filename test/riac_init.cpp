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

#include <iostream>

#include "caf/config.hpp"

#define CAF_SUITE riac_init
#include "caf/test/unit_test.hpp"

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "caf/riac/all.hpp"

using namespace caf;

using std::cout;
using std::endl;

void run_probe(int argc, char** argv, uint16_t port) {
  actor_system_config cfg{argc, argv};
  cfg.nexus_host = "127.0.0.1";
  cfg.nexus_port = port;
  cfg.load<io::middleman>()
     .load<riac::probe>();
  actor_system system{cfg};
  CAF_REQUIRE(system.probe().connected());
  // system connects to the node during startup (ctor),
  // then closes it on shutdown (dtor)
}

void run_nexus(int argc, char** argv) {
  actor_system_config cfg{argc, argv};
  cfg.load<io::middleman>();
  riac::add_message_types(cfg);
  actor_system system{cfg};
  scoped_actor self{system};
  auto nexus = system.spawn<riac::nexus>(true);
  self->send(nexus, riac::add_listener{self});
  auto port = system.middleman().publish(nexus, 0);
  CAF_REQUIRE(port);
  CAF_MESSAGE("published nexus at port " << *port);
  std::thread child{[=] { run_probe(argc, argv, *port); }};
  self->receive(
    [&](const riac::node_info&, const actor&) {
      CAF_MESSAGE("received node info of probe");
    }
  );
  self->receive(
    [&](const riac::node_disconnected&) {
      CAF_MESSAGE("probe disconnected");
    }
  );
  anon_send_exit(nexus, exit_reason::kill);
  child.join();
}

CAF_TEST(riac_init) {
  auto argc = test::engine::argc();
  auto argv = test::engine::argv();
  run_nexus(argc, argv);
  /*
  CAF_MESSAGE("this node is: " << to_string(detail::singletons::get_node_id()));
  uint16_t port = 0;
  auto r = message_builder(argv, argv + argc).extract_opts({
    {"nexus,n", "run nexus in server mode", port},
    {"probe,p", "run as probe", port}
  });
  if (! r.error.empty() || r.opts.count("help") > 0 || ! r.remainder.empty()) {
    CAF_TEST_ERROR(r.error << "\n\n" << r.helptext);
    return;
  }
  riac::announce_message_types();
  if (r.opts.count("nexus") > 0) {
    CAF_MESSAGE("don't run remote actor (server mode)");
    run_nexus(false, port);
  } else if (r.opts.count("probe") > 0) {
    run_probe(port);
  } else {
    run_nexus(true, 0);
  }
  await_all_actors_done();
  shutdown();
  */
}
