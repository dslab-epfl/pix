#include "policer_state.h"
#include "lib/nf_util.h"
#include <stdlib.h>
#ifdef KLEE_VERIFICATION
#include "lib/stubs/containers/double-chain-stub-control.h"
#include "lib/stubs/containers/map-stub-control.h"
#include "lib/stubs/containers/vector-stub-control.h"
#include "lib/stubs/time_stub_control.h"
#endif // KLEE_VERIFICATION

struct State *allocated_nf_state = NULL;

struct State *alloc_state(int max_flows) {
  if (allocated_nf_state != NULL)
    return allocated_nf_state;
  struct State *ret = malloc(sizeof(struct State));
  if (ret == NULL)
    return NULL;
  ret->fm = NULL;
  if (map_allocate(policer_flow_eq, policer_flow_hash, max_flows, &(ret->fm)) == 0)
    return NULL;
  ret->fk = NULL;
  if (vector_allocate(sizeof(struct Bucket), max_flows, policer_flow_allocate,
                      &(ret->fk)) == 0)
    return NULL;
  ret->fv = NULL;
  if (vector_allocate(sizeof(struct Bucket), max_flows, policer_flow_allocate,
                      &(ret->fv)) == 0)
    return NULL;
  ret->heap = NULL;
  if (dchain_allocate(max_flows, &(ret->heap)) == 0)
    return NULL;
  ret->max_flows = max_flows;

  DS_INIT(map, ret->fm, "policer_flowtable", "pkt.flow")
  DS_INIT(dchain, ret->heap, "nat_flowtable", "pkt.flow")
  DS_INIT(vector, ret->fv, "vector", "pkt.flow.bucket")
#ifdef KLEE_VERIFICATION
  map_set_key_size(ret->fm, sizeof(uint32_t));
  vector_set_entry_condition(ret->fv, policer_bucket_sanity_check, ret);
#endif // KLEE_VERIFICATION
  allocated_nf_state = ret;
  return ret;
}

#ifdef KLEE_VERIFICATION
void loop_reset(struct Map* fm, struct Vector* fk, struct Vector*fv, struct DoubleChain* heap, int max_flows, time_t* time){
  map_reset(fm);
  vector_reset(fk);
  vector_reset(fv);
  dchain_reset(heap,max_flows);
  *time = restart_time();

}

void nf_loop_iteration_border(unsigned lcore_id, time_t time) {
  loop_reset(allocated_nf_state->fm, allocated_nf_state->fk, allocated_nf_state->fv, allocated_nf_state->heap, allocated_nf_state->max_flows, &time);
}
#endif // KLEE_VERIFICATION
