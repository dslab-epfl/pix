#include "nat-state.h"
#include "lib/nf_util.h"
#include <stdlib.h>
#ifdef KLEE_VERIFICATION
#include "lib/stubs/containers/double-chain-stub-control.h"
#include "lib/stubs/containers/map-stub-control.h"
#include "lib/stubs/containers/vector-stub-control.h"
#endif // KLEE_VERIFICATION
struct State *allocated_nf_state = NULL;
bool flow_consistency(void *value, void *state);

struct State *alloc_state(int max_flows, int start_port, uint32_t ext_ip,
                          uint32_t nat_device) {
  if (allocated_nf_state != NULL)
    return allocated_nf_state;
  struct State *ret = malloc(sizeof(struct State));
  if (ret == NULL)
    return NULL;
  ret->fm = NULL;
  if (map_allocate(FlowId_eq, FlowId_hash, max_flows, &(ret->fm)) == 0)
    return NULL;
  ret->fv = NULL;
  if (vector_allocate(sizeof(struct FlowId), max_flows, FlowId_allocate,
                      &(ret->fv)) == 0)
    return NULL;
  ret->heap = NULL;
  if (dchain_allocate(max_flows, &(ret->heap)) == 0)
    return NULL;
  ret->max_flows = max_flows;
  ret->start_port = start_port;
  ret->ext_ip = ext_ip;
  ret->nat_device = nat_device;

  DS_INIT(map, ret->fm, "nat_flowtable", "pkt.flow")
  DS_INIT(dchain, ret->heap, "nat_flowtable", "pkt.flow")
  DS_INIT(vector, ret->fv, "vector", "")
#ifdef KLEE_VERIFICATION
  map_set_key_size(ret->fm, sizeof(struct FlowId));
  vector_set_entry_condition(ret->fv, flow_consistency, ret);
#endif // KLEE_VERIFICATION
  allocated_nf_state = ret;
  return ret;
}

#ifdef KLEE_VERIFICATION
void nf_loop_iteration_border(unsigned lcore_id, time_t time) {
  loop_iteration_border(
      &allocated_nf_state->fm, &allocated_nf_state->fv,
      &allocated_nf_state->heap, allocated_nf_state->max_flows,
      allocated_nf_state->start_port, allocated_nf_state->ext_ip,
      allocated_nf_state->nat_device, lcore_id, time);
}

#endif // KLEE_VERIFICATION
