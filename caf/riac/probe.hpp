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

#ifndef CAF_RIAC_PROBE_HPP
#define CAF_RIAC_PROBE_HPP

#include <string>
#include <cstdint>

#include "caf/actor_system.hpp"

#include "caf/riac/nexus.hpp"

namespace caf {
namespace riac {

class probe : public actor_system::module {
public:
  probe(actor_system& sys);

  void start() override;

  void stop() override;

  void init(actor_system_config&) override;

  id_t id() const override;

  bool connected();

  void* subtype_ptr() override;

  static actor_system::module* make(actor_system&, detail::type_list<>);

private:
  actor_system& system_;
  std::string nexus_host_;
  uint16_t nexus_port_;
  nexus_type uplink_;
};

} // namespace riac
} // namespace caf

#endif // CAF_RIAC_PROBE_HPP
