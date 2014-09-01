/******************************************************************************
 *                       ____    _    _____                                   *
 *                      / ___|  / \  |  ___|    C++                           *
 *                     | |     / _ \ | |_       Actor                         *
 *                     | |___ / ___ \|  _|      Framework                     *
 *                      \____/_/   \_|_|                                      *
 *                                                                            *
 * Copyright (C) 2011 - 2014                                                  *
 * Dominik Charousset <dominik.charousset (at) haw-hamburg.de>                *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENCE_ALTERNATIVE.       *
 *                                                                            *
 * If you did not receive a copy of the license files, see                    *
 * http://opensource.org/licenses/BSD-3-Clause and                            *
 * http://www.boost.org/LICENSE_1_0.txt.                                      *
 ******************************************************************************/

#ifndef CAF_PROBE_IF_HPP
#define CAF_PROBE_IF_HPP

#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <iostream>

#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>


namespace caf {
namespace probe {
namespace interface {

std::vector<std::string> ifnames();
std::string hw_addr(const std::string& name);
std::string ipv4_addr(const std::string& name);
std::string ipv6_addr(const std::string& name);

} // namespace interface
} // namespace probe
} // namespace caf

#endif // CAF_PROBE_IF_HPP
