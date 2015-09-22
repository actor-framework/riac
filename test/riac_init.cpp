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

#include "caf/detail/run_sub_unit_test.hpp"

using namespace caf;

using std::cout;
using std::endl;

void run_probe(uint16_t port) {
  CAF_CHECK(riac::init_probe("localhost", port));
}

void run_nexus(bool launch_probe, uint16_t port) {
  scoped_actor self;
  auto nexus = spawn<riac::nexus>();
  self->send(nexus, riac::add_listener{self});
  port = io::typed_publish(nexus, port);
  CAF_MESSAGE("published nexus at port " << port);
  std::thread child;
  if (launch_probe) {
    child = detail::run_sub_unit_test(self,
                                      test::engine::path(),
                                      test::engine::max_runtime(),
                                      CAF_XSTR(CAF_SUITE),
                                      false,
                                      {"--probe=" + std::to_string(port)});
  }
  self->receive(
    [&](const riac::node_info&) {
      CAF_MESSAGE("received node info of probe");
    }
  );
  self->receive(
    [&](const riac::node_disconnected&) {
      CAF_MESSAGE("probe disconnected");
    }
  );
  if (launch_probe) {
    child.join();
    self->receive(
      [](const std::string& output) {
        cout << endl << endl << "*** output of client program ***"
             << endl << output << endl;
      }
    );
  }
  anon_send_exit(nexus, exit_reason::kill);
}

CAF_TEST(riac_init) {
  auto argv = test::engine::argv();
  auto argc = test::engine::argc();
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
}
