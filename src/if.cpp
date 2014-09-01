#include "caf/probe/if.hpp"

using namespace std;

namespace caf {
namespace probe {
namespace interface {

vector<string> ifnames() {
  vector<string> ifnames;
  struct if_nameindex *if_ni;
  struct if_nameindex *i;
  if_ni = if_nameindex();
  if (if_ni == NULL) {
    return ifnames;
  }
  for (i = if_ni; !(i->if_index == 0 && i->if_name == NULL); i++) {
    ifnames.push_back(string(i->if_name));
  }
  if_freenameindex(if_ni);
  return ifnames;
}

// TODO: find other way to get mac without sockets
string hw_addr(const string& name) {
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
    // TODO: use c++ cast (reinterpret?)
    unsigned char sign = (unsigned char) buffer.ifr_hwaddr.sa_data[i];
    ss << setfill('0') << setw(2) << (int) sign;
    if (i != 5) {
      ss << ':';
    }
  }
  return ss.str();
}

string ipv4_addr(const string& name) {
  struct ifaddrs* ifa     = NULL;
  struct ifaddrs* ifEntry = NULL;
  void*           ipPtr  = NULL;
  string          invalid_ip = "0.0.0.0";
  char addressBuffer[INET6_ADDRSTRLEN];
  if (getifaddrs(&ifa)) {
    return invalid_ip;
  }
  for (ifEntry = ifa; ifEntry != NULL; ifEntry = ifEntry->ifa_next) {
    if (strcmp(ifEntry->ifa_name, name.c_str()) == 0
    &&  ifEntry->ifa_addr->sa_family == AF_INET
    &&  ifEntry->ifa_addr->sa_data != NULL) {
      // TODO: use c++ casts
      ipPtr = &((struct sockaddr_in*)ifEntry->ifa_addr)->sin_addr;
      break;
    }
  }
  if (ipPtr == NULL) {
    return invalid_ip;
  }
  // format to string
  inet_ntop(ifEntry->ifa_addr->sa_family,
            ipPtr,
            addressBuffer,
            sizeof(addressBuffer));
  freeifaddrs(ifa);
  return string(addressBuffer);
}

string ipv6_addr(const string& name) {
  struct ifaddrs* ifa     = NULL;
  struct ifaddrs* ifEntry = NULL;
  void*           ipPtr  = NULL;
  string          invalid_ip = "::1";
  char addressBuffer[INET6_ADDRSTRLEN];
  if (getifaddrs(&ifa)) {
    return invalid_ip;
  }
  for (ifEntry = ifa; ifEntry != NULL; ifEntry = ifEntry->ifa_next) {
    if (strcmp(ifEntry->ifa_name, name.c_str()) == 0
    &&  ifEntry->ifa_addr->sa_family == AF_INET6
    &&  ifEntry->ifa_addr->sa_data != NULL) {
      // TODO: use c++ casts
      ipPtr = &((struct sockaddr_in6 *)ifEntry->ifa_addr)->sin6_addr;
      break;
    }
  }
  if (ipPtr == NULL) {
    return invalid_ip;
  }
  // format to string
  inet_ntop(ifEntry->ifa_addr->sa_family,
            ipPtr,
            addressBuffer,
            sizeof(addressBuffer));
  freeifaddrs(ifa);
  return string(addressBuffer);
}

} // namespace interface
} // namespace probe
} // namespace caf

