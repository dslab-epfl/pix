#ifndef _DOUBLE_MAP_STUB_CONTROL_H_INCLUDED_
#define _DOUBLE_MAP_STUB_CONTROL_H_INCLUDED_
#include "include_ignored_by_verifast.h"
#include "lib/containers/double-map.h"
#include "str-descr.h"

#ifdef _NO_VERIFAST_
extern int Num_bucket_traversals;
extern int Num_hash_collisions;
extern int recent_flow;
#endif //_NO_VERIFAST_

typedef int entry_condition(void *key_a, void *key_b, int index, void *value);

#define ALLOW(map) klee_allow_access((map), sizeof(struct DoubleMap))
#define DENY(map)                                                              \
  klee_forbid_access((map), sizeof(struct DoubleMap),                          \
                     "Externally for the double map model code")

#define prealloc_size (256)

struct DoubleMap {
  /* Concrete state */
  int value_size_g;
  int key_a_size_g;
  int key_b_size_g;

  uint8_t *value; //[prealloc_size]; - for klee_make_symbolic
  int has_this_key;
  int entry_claimed;
  int *allocated_indexp; // must be a separate memory reg for klee_make_symbolic
  int occupancy;
  int capacity;

  entry_condition *ent_cond;

  map_keys_equality *eq_a_g;
  map_keys_equality *eq_b_g;
  dmap_extract_keys *dexk_g;
  dmap_pack_keys *dpk_g;
};

void dmap_set_entry_condition(struct DoubleMap *map, entry_condition *cond);
//@ requires true;
//@ ensures true;

void dmap_reset(struct DoubleMap *map, int capacity);

void dmap_increase_occupancy(struct DoubleMap *map, int change);

void dmap_decrease_occupancy(struct DoubleMap *map, int change);

int *dmap_occupancy_p(struct DoubleMap *map);

void dmap_lowerbound_on_occupancy(struct DoubleMap *map, int lower_bound);

#endif //_DOUBLE_MAP_STUB_CONTROL_H_INCLUDED_
