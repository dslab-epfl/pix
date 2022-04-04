#include "double-map-stub-control.h"
#include "klee/klee.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int Num_bucket_traversals;
int Num_hash_collisions;
int recent_flow;
char *prefix; /* For tracing */

void dmap_set_entry_condition(struct DoubleMap *map, entry_condition *c) {
  ALLOW(map);
  map->ent_cond = c;
  DENY(map);
}

void dmap_increase_occupancy(struct DoubleMap *map, int change) {
  ALLOW(map);
  map->occupancy = map->occupancy + change;
  klee_assert(map->occupancy <= 65536);
  DENY(map);
}

void dmap_decrease_occupancy(struct DoubleMap *map, int change) {
  ALLOW(map);
  map->occupancy = map->occupancy - change;
  klee_assert(map->occupancy >= 0);
  DENY(map);
}

int *dmap_occupancy_p(struct DoubleMap *map) { return &map->occupancy; }

void dmap_lowerbound_on_occupancy(struct DoubleMap *map, int lower_bound) {
  ALLOW(map);
  klee_assume(map->occupancy >= lower_bound);
  DENY(map);
}

int dmap_allocate(map_keys_equality eq_a, map_key_hash hsh_a,
                  map_keys_equality eq_b, map_key_hash hsh_b, int value_size,
                  uq_value_copy v_cpy, uq_value_destr v_destr,
                  dmap_extract_keys dexk, dmap_pack_keys dpk, int capacity,
                  struct DoubleMap **map_out) {

  int allocation_succeeded = klee_int("dmap_allocation_succeeded");
  *map_out = malloc(sizeof(struct DoubleMap));
  klee_possibly_havoc(*map_out, sizeof(struct DoubleMap), "map_out");

  /* Tracing function and necessary variable(s) */

  klee_trace_ret();
  /* For contract dependencies? */
  klee_trace_param_fptr(eq_a, "eq_a");
  klee_trace_param_fptr(hsh_a, "hsh_a");
  klee_trace_param_fptr(eq_b, "eq_b");
  klee_trace_param_fptr(hsh_b, "hsh_b");
  klee_trace_param_fptr(v_cpy, "v_cpy");
  klee_trace_param_fptr(v_destr, "v_destr");
  klee_trace_param_fptr(dexk, "dexk");
  klee_trace_param_fptr(dpk, "dpk");
  /* To differentiate between different maps */
  klee_trace_param_ptr_directed(map_out, sizeof(struct DoubleMap *), "map_out",
                                TD_OUT);

  if (allocation_succeeded & (*map_out != NULL)) {
    memset(*map_out, 0, sizeof(struct DoubleMap));
    klee_assert(value_size < prealloc_size);

    (*map_out)->eq_a_g = eq_a;
    (*map_out)->eq_b_g = eq_b;
    (*map_out)->dexk_g = dexk;
    (*map_out)->dpk_g = dpk;

    (*map_out)->value_size_g = value_size;
    (*map_out)->entry_claimed = 0;

    /* Hack */
    (*map_out)->key_a_size_g = 17;
    (*map_out)->key_b_size_g = 17;

    /* Setting up symbolic state */

    (*map_out)->has_this_key = klee_int("dmap_has_this_key");
    Num_bucket_traversals = klee_int("Num_bucket_traversals");
    Num_hash_collisions = klee_int("Num_hash_collisions");
    recent_flow = 0;
    if ((*map_out)->has_this_key)
      recent_flow = klee_int("recent_flow");

    // (*map_out)->has_this_key ?klee_int("recent_flow") : 0;

    /* Need a separate memory record for allocated_indexp to be able
      to mark it symbolic. */
    (*map_out)->allocated_indexp =
        malloc(sizeof(*(*map_out)->allocated_indexp));
    klee_possibly_havoc((*map_out)->allocated_indexp,
                        sizeof(*(*map_out)->allocated_indexp),
                        "dmap_allocated_index");
    klee_make_symbolic((*map_out)->allocated_indexp,
                       sizeof(*(*map_out)->allocated_indexp),
                       "dmap_allocated_index");
    klee_assume(0 <= *(*map_out)->allocated_indexp);
    klee_assume(*(*map_out)->allocated_indexp < capacity);

    /* Create a separate memory region to allow klee_make_symbolic */
    (*map_out)->value = malloc(prealloc_size);
    klee_possibly_havoc((*map_out)->value, prealloc_size, "dmap_value");
    klee_make_symbolic((*map_out)->value, prealloc_size, "dmap_value");
    (*map_out)->occupancy = klee_range(0, capacity, "dmap_occupancy");
    (*map_out)->capacity = capacity;

    /* Tracing other variable(s) */

    TRACE_VAL((uint32_t)(*map_out), "dmap", _u32)

    TRACE_VAR((*map_out)->capacity, "dmap_capacity")
    TRACE_VAR((*map_out)->occupancy, "dmap_occupancy")

    /* Do not assume the ent_cond here, because depending on what comes next,
       we may change the key_a, key_b or value. we assume the condition after
       that change. */

    DENY(*map_out);
    return 1;
  }
  return 0;
}

int dmap_get_a(struct DoubleMap *map, void *key, int *index) {
  ALLOW(map);

  /* Tracing function and necessary variable(s) */

  klee_trace_ret();
  /* To differentiate between different maps */
  klee_trace_param_i32((uint32_t)map, "map");

  TRACE_VAL((uint32_t)(map), "dmap", _u32)
  TRACE_VAR(map->has_this_key, "dmap_has_this_key")
  TRACE_VAR(Num_bucket_traversals, "Num_bucket_traversals")
  TRACE_VAR(Num_hash_collisions, "Num_hash_collisions")
  TRACE_VAR(recent_flow, "recent_flow")

  if (map->has_this_key) {
    klee_assert(!map->entry_claimed);
    void *key_a = NULL;
    void *key_b = NULL;
    map->dexk_g(map->value, &key_a, &key_b);

    klee_assume(map->eq_a_g(key_a, key));
    if (map->ent_cond)
      klee_assume(
          map->ent_cond(key_a, key_b, *map->allocated_indexp, map->value));
    map->dpk_g(map->value, key_a, key_b);
    map->entry_claimed = 1;
    *index = *map->allocated_indexp;

    PERF_MODEL_BRANCH(recent_flow, 1, 2)
    DENY(map);
    return 1;
  }
  klee_assert(!recent_flow);
  recent_flow = 1;
  DENY(map);
  return 0;
}

int dmap_get_b(struct DoubleMap *map, void *key, int *index) {
  ALLOW(map);

  /* Tracing function and necessary variable(s) */
  klee_trace_ret();
  /* To differentiate between different maps */
  klee_trace_param_i32((uint32_t)map, "map");

  TRACE_VAL((uint32_t)(map), "dmap", _u32)
  TRACE_VAR(map->has_this_key, "dmap_has_this_key")
  TRACE_VAR(Num_bucket_traversals, "Num_bucket_traversals")
  TRACE_VAR(Num_hash_collisions, "Num_hash_collisions")
  TRACE_VAR(recent_flow, "recent_flow")

  if (map->has_this_key) {
    klee_assert(!map->entry_claimed);
    void *key_a = NULL;
    void *key_b = NULL;
    map->dexk_g(map->value, &key_a, &key_b);
    klee_assume(map->eq_b_g(key_b, key));
    if (map->ent_cond)
      klee_assume(
          map->ent_cond(key_a, key_b, *map->allocated_indexp, map->value));
    map->dpk_g(map->value, key_a, key_b);
    map->entry_claimed = 1;
    *index = *map->allocated_indexp;

    PERF_MODEL_BRANCH(recent_flow, 1, 2)
    DENY(map);
    return 1;
  }
  klee_assert(!recent_flow);
  recent_flow = 1;
  DENY(map);
  return 0;
}

int dmap_put(struct DoubleMap *map, void *value_, int index) {
  ALLOW(map);

  /* Tracing function and necessary variable(s) */
  klee_trace_ret();
  /* To differentiate between different maps */
  klee_trace_param_i32((uint32_t)map, "map");

  TRACE_VAL((uint32_t)(map), "dmap", _u32)
  TRACE_VAR(map->has_this_key, "dmap_has_this_key")
  TRACE_VAR(Num_bucket_traversals, "Num_bucket_traversals")
  TRACE_VAR(Num_hash_collisions, "Num_hash_collisions")
  TRACE_VAR(recent_flow, "recent_flow")

  /* Can not ever fail, because index is guaranteed to point to the available
     slot, therefore the map can not be full at this point.
     Always returns 1. */
  if (map->entry_claimed) {
    klee_assert(*map->allocated_indexp == index);
  }
  memcpy(map->value, value_, map->value_size_g);
  void *key_a = 0;
  void *key_b = 0;
  map->dexk_g(map->value, &key_a, &key_b);
  // This must be provided by the caller, since it his responsibility
  // to fulfill the value by the same index:
  klee_assert(map->ent_cond == NULL ||
              map->ent_cond(key_a, key_b, index, map->value));
  klee_assume(map->occupancy < map->capacity);
  map->dpk_g(map->value, key_a, key_b);
  map->entry_claimed = 1;
  *map->allocated_indexp = index;

  PERF_MODEL_BRANCH(recent_flow, 1, 2)

  DENY(map);
  dmap_increase_occupancy(map, 1);
  return 1;
}

int dmap_erase(struct DoubleMap *map, int index) {
  klee_assert(map != NULL);
  ALLOW(map);
  klee_trace_param_i32((uint32_t)map, "map");
  klee_trace_param_i32(index, "index");

  klee_assert(0); // This model does not support erasure.
  DENY(map);
  return 0;
}

void dmap_get_value(struct DoubleMap *map, int index, void *value_out) {
  ALLOW(map);

  /* Tracing function and necessary variable(s) */
  klee_trace_ret();
  /* To differentiate between different maps */
  klee_trace_param_i32((uint32_t)map, "map");

  TRACE_VAL((uint32_t)(map), "dmap", _u32)
  TRACE_VAR(recent_flow, "recent_flow")

  if (map->entry_claimed) {
    klee_assert(index == *map->allocated_indexp);
  } else {
    *map->allocated_indexp = index;
    map->entry_claimed = 1;
  }
  memcpy(value_out, map->value, map->value_size_g);
  PERF_MODEL_BRANCH(recent_flow, 1, 2)
  DENY(map);
}

int dmap_size(struct DoubleMap *map) {
  klee_assert(0); // This model does not support size requests.
  return -1;
}

void dmap_reset(struct DoubleMap *map, int capacity) {
  ALLOW(map);
  map->entry_claimed = 0;
  map->occupancy = klee_range(0, capacity + 1, "dmap_occupancy");
  map->allocated_indexp = malloc(sizeof(int));
  klee_make_symbolic(map->allocated_indexp, sizeof(int),
                     "allocated_index_reset");
  klee_assume(0 <= *map->allocated_indexp);
  klee_assume(*map->allocated_indexp < capacity);
  klee_assume(!(map->has_this_key == 0) | (recent_flow == 0));
  DENY(map);
}
