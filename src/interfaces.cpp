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

#include "caf/riac/interfaces.hpp"

#include <memory>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "caf/optional.hpp"
#include "caf/detail/get_mac_addresses.hpp"

using namespace std;

namespace caf {
namespace riac {

in6_addr* fetch_in_addr(sockaddr_in6* addr) {
  return &(addr->sin6_addr);
}

in_addr* fetch_in_addr(sockaddr_in* addr) {
  return &(addr->sin_addr);
}

template <class SockaddrType, int Family>
void add_addr(ifaddrs* first, std::vector<std::string>& res) {
  auto addr = reinterpret_cast<SockaddrType*>(first->ifa_addr);
  auto in_addr = fetch_in_addr(addr);
  char address_buffer[INET6_ADDRSTRLEN];
  inet_ntop(Family, in_addr, address_buffer, INET6_ADDRSTRLEN);
  res.push_back(address_buffer);
}

interfaces_map interfaces() {
  interfaces_map res;
  // fetch Ethernet addresses
  auto mac_addresses = detail::get_mac_addresses();
  for (auto& i : mac_addresses) {
    res[i.interface_name][protocol::ethernet].push_back(i.ethernet_address);
  }
  // fetch IPv4 and IPv6 addresses
  ifaddrs* tmp = nullptr;
  if (getifaddrs(&tmp) != 0) {
    perror("getifaddrs");
    return res;
  }
  unique_ptr<ifaddrs, decltype(freeifaddrs)*> ifs{tmp, freeifaddrs};
  for (auto i = ifs.get(); i != nullptr; i = i->ifa_next) {
    switch (i->ifa_addr->sa_family) {
      default:
        // AF_PROTOCOL is being ignored
        break;
      case AF_INET:
        add_addr<sockaddr_in, AF_INET>(i, res[i->ifa_name][protocol::ipv4]);
        break;
      case AF_INET6:
        add_addr<sockaddr_in6, AF_INET6>(i, res[i->ifa_name][protocol::ipv6]);
        break;
    }
  }
  return res;
}

} // namespace riac
} // namespace caf
