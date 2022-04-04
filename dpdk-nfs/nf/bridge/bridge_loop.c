#include "bridge_loop.h"
#include "lib/stubs/containers/double-chain-stub-control.h"
#include "lib/stubs/containers/map-stub-control.h"
#include "lib/stubs/containers/vector-stub-control.h"
#include "lib/stubs/time_stub_control.h"
#include <klee/klee.h>

void bridge_loop_iteration_assumptions(
    struct DoubleChain **dyn_heap, struct Map **dyn_map,
    struct Vector **dyn_keys, struct Vector **dyn_vals, struct Map **st_map,
    struct Vector **st_vec, uint32_t capacity, time_t time) {
  dchain_reset(*dyn_heap, capacity);
  map_reset(*dyn_map);
  vector_reset(*dyn_keys);
  vector_reset(*dyn_vals);
#ifndef NO_STATIC_MAPPING
  map_reset(*st_map);
#endif //! NO_STATIC_MAPPING
}

void bridge_loop_invariant_consume(struct DoubleChain **dyn_heap,
                                   struct Map **dyn_map,
                                   struct Vector **dyn_keys,
                                   struct Vector **dyn_vals,
                                   struct Map **st_map, struct Vector **st_vec,
                                   uint32_t capacity, time_t time,
                                   uint32_t dev_count) {
  klee_trace_ret();
}

void bridge_loop_invariant_produce(struct DoubleChain **dyn_heap,
                                   struct Map **dyn_map,
                                   struct Vector **dyn_keys,
                                   struct Vector **dyn_vals,
                                   struct Map **st_map, struct Vector **st_vec,
                                   uint32_t capacity, time_t *time,
                                   uint32_t dev_count) {
  klee_trace_ret();
  bridge_loop_iteration_assumptions(dyn_heap, dyn_map, dyn_keys, dyn_vals,
                                    st_map, st_vec, capacity, *time);
  *time = restart_time();
}

void bridge_loop_iteration_border(struct DoubleChain **dyn_heap,
                                  struct Map **dyn_map,
                                  struct Vector **dyn_keys,
                                  struct Vector **dyn_vals, struct Map **st_map,
                                  struct Vector **st_vec, uint32_t capacity,
                                  time_t time, uint16_t dev_count) {
  bridge_loop_invariant_consume(dyn_heap, dyn_map, dyn_keys, dyn_vals, st_map,
                                st_vec, capacity, time, dev_count);
  bridge_loop_invariant_produce(dyn_heap, dyn_map, dyn_keys, dyn_vals, st_map,
                                st_vec, capacity, &time, dev_count);
}

void bridge_loop_iteration_end(struct DoubleChain **dyn_heap,
                               struct Map **dyn_map, struct Vector **dyn_keys,
                               struct Vector **dyn_vals, struct Map **st_map,
                               struct Vector **st_vec, uint32_t capacity,
                               time_t time, uint16_t dev_count) {
  bridge_loop_invariant_consume(dyn_heap, dyn_map, dyn_keys, dyn_vals, st_map,
                                st_vec, capacity, time, dev_count);
  bridge_loop_invariant_produce(dyn_heap, dyn_map, dyn_keys, dyn_vals, st_map,
                                st_vec, capacity, &time, dev_count);
}
