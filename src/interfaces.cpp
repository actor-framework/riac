#include "caf/probe/if.hpp"

#include <memory>
#include <errno.h>

#include "caf/optional.hpp"

namespace {

constexpr const char invalid_ipv4[] = "0.0.0.0";
constexpr const char invalid_ipv6[] = "::1";

} // namespace <anonymous>

using namespace std;

namespace caf {
namespace probe {


string get_invalid_ip(sockaddr_in) {
   return invalid_ipv4;
}

string get_invalid_ip(sockaddr_in6) {
   return invalid_ipv6;
}

vector<string> interface_names() {
  vector<string> ifnames;
  unique_ptr<struct if_nameindex, decltype(if_freenameindex)*> if_ni {
    if_nameindex(), if_freenameindex
  };
  if (if_ni.get() == nullptr) {
    return ifnames;
  }
  auto name_ptr = if_ni.get();
  for (; name_ptr->if_index != 0 || name_ptr->if_name != nullptr; name_ptr++) {
    ifnames.push_back(string(name_ptr->if_name));
  }
  return ifnames;
}

// TODO: find other way to get mac without sockets
// TODO: use unique ptr
vector<string> hw_addr(const string& name) {
  // get hwaddr in signs
  int s;
  struct ifreq buffer;
  s = socket(PF_INET, SOCK_DGRAM, 0);
  memset(&buffer, 0x00, sizeof(buffer));
  strcpy(buffer.ifr_name, name.c_str());
  ioctl(s, SIOCGIFHWADDR, &buffer);
  close(s);
  // format to string
  stringstream ss;
  ss << hex;
  for (int i = 0; i < 6; i++) {
    unsigned char sign =
                  static_cast<unsigned char>(buffer.ifr_hwaddr.sa_data[i]);
    ss << setfill('0') << setw(2) << (int) sign;
    if (i != 5) {
      ss << ':';
    }
  }
  return {ss.str()};
}

optional<unique_ptr<ifaddrs, decltype(freeifaddrs)*>> getifaddrs_uniqueptr() {
  ifaddrs* tmp = nullptr;
  if (getifaddrs(&tmp)) {
    return none;
  }
  return {{tmp, freeifaddrs}};
}

// TODO: rename
in6_addr* get_ip_in_bit(sockaddr_in6* addr) {
  return &(addr->sin6_addr);
}

in_addr* get_ip_in_bit(sockaddr_in* addr) {
  return &(addr->sin_addr);
}

template <class SockaddrType, int Family>
vector<string> fill_ips(ifaddrs* first, const string& name) {
  vector<string> accu;
  for (; first != nullptr; first = first->ifa_next) {
    if (strcmp(first->ifa_name, name.c_str()) == 0
        && first->ifa_addr->sa_family == Family
        && first->ifa_addr->sa_data != nullptr) {
      auto addr = reinterpret_cast<SockaddrType*>(first->ifa_addr);
      auto ip_in_bit_ptr = get_ip_in_bit(addr);
      char address_buffer[INET6_ADDRSTRLEN];
      inet_ntop(Family, ip_in_bit_ptr, address_buffer, sizeof(address_buffer));
      auto ip = string(address_buffer);
      accu.push_back(ip);
    }
  }
  if (accu.empty()) {
    SockaddrType tmp;
    accu.push_back(get_invalid_ip(tmp));
  }
  return accu;
}

interfaces get_interfaces() {
  interfaces accu;
  auto if_names = interface_names();
  for (auto interface_name : if_names) {
    std::map<interface_type, std::vector<string>> prop;
    prop.emplace(interface_type::ethernet, hw_addr(interface_name));
    auto ifa = getifaddrs_uniqueptr();
    if (ifa) {
      auto ipv4s = fill_ips<sockaddr_in, AF_INET>(ifa->get(), interface_name);
      prop.emplace(interface_type::ipv4, ipv4s);
      auto ipv6s = fill_ips<sockaddr_in6, AF_INET6>(ifa->get(), interface_name);
      prop.emplace(interface_type::ipv6, ipv6s);
      accu.emplace(interface_name, prop);
    }
  }
  return accu;
}

} // namespace probe
} // namespace caf

