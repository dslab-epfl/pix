#include "lib/expirator.h"
#include "lib/stubs/containers/double-chain-stub-control.h"
#include "lib/stubs/containers/double-map-stub-control.h"
#include "lib/stubs/containers/map-stub-control.h"
#include <klee/klee.h>

char *prefix; /* For tracing */
int expire_items(struct DoubleChain *chain, struct DoubleMap *map,
                 time_t time) {
  ALLOW(map);
  klee_trace_ret();
  /* To differentiate between different flowmanager */
  klee_trace_param_i32((uint32_t)chain, "chain");
  klee_trace_param_i32((uint32_t)map, "map");

  int *occupancyp = dmap_occupancy_p(map);
  DENY(map);

  TRACE_VAL((uint32_t)(map), "dmap", _u32)
  TRACE_VAR(Num_bucket_traversals, "Num_bucket_traversals")
  TRACE_VAR(Num_hash_collisions, "Num_hash_collisions")
  TRACE_VAR(recent_flow, "recent_flow")

  int nfreed = klee_int("number_of_freed_flows");
  TRACE_VAR(nfreed, "expired_flows")

  klee_assume(0 <= nfreed);
  dmap_lowerbound_on_occupancy(map, nfreed);
  dchain_make_space(chain, nfreed);
  // Tell dchain model that we freed some indexes here
  dmap_decrease_occupancy(map, nfreed);
  ds_path_1();
  return nfreed;
}

int expire_items_single_map(struct DoubleChain *chain, struct Vector *vector,
                            struct Map *map, time_t time) {
  klee_trace_ret();
  /* To differentiate between different flowmanagers */
  klee_trace_param_i32((uint32_t)chain, "chain");
  klee_trace_param_i32((uint32_t)vector, "vector");
  klee_trace_param_i32((uint32_t)map, "map");

  TRACE_VAL((uint32_t)(map), "map", _u32)

  int nfreed = klee_int("number_of_freed_flows");
  klee_assume(0 <= nfreed);
  dchain_make_space(chain, nfreed);
  TRACE_VAR(map->capacity, "map_capacity")
  TRACE_VAR(map->occupancy, "map_occupancy")
  TRACE_VAR(map->Num_bucket_traversals, "Num_bucket_traversals")
  TRACE_VAR(map->Num_hash_collisions, "Num_hash_collisions")
  TRACE_VAR(nfreed, "expired_flows")
  TRACE_VAR(map->Num_hash_collisions, "Num_hash_collisions")
  TRACE_FPTR(map->khash, "map_hash")
  TRACE_FPTR(map->keq, "map_key_eq")
  ds_path_1();
  return nfreed;
}