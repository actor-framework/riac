#include <vector>
#include <cstdint>

#include "caf/node_id.hpp"

namespace caf {
namespace probe_event {

// send periodically from ActorProbe to ActorNexus
struct ram_usage {
  uint64_t in_use;
  uint64_t available;
};
 
// send periodically from ActorProbe to ActorNexus
struct work_load {
  uint8_t cpu_load;      // in percent, i.e., 0-100
  uint64_t num_processes;
  uint64_t num_actors;
};
 
struct cpu_info {
  uint64_t num_cores;
  uint64_t mhz_per_core;
};
 
// send on connect from ActorProbe to ActorNexus
struct node_info {
  node_id id;
  std::vector<cpu_info> cpu;
};
 
// send from ActorProbe to ActorNexus whenever from learns a new direct or 
// indirect route to another node
struct new_route {
  node_id from;
  node_id to;
  bool is_direct;
};
 
struct new_message {
  node_id source_node;
  node_id dest_node;
  actor_id source_actor;
  actor_id dest_actor;
  message msg;
};

inline void announce_all() {
}

} // namespace probe_event
} // namespace caf

