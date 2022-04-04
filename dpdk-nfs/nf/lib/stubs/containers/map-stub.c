#include "klee/klee.h"
#include "lib/containers/map.h"
#include "map-stub-control.h"
#include <stdlib.h>

char *prefix; /* For tracing */

/* For tracing map_key_cached */
char *base_key_cached = "map_key_cached";
char *suffix_key_cached = "_occurence";
char *final_key_cached;

__attribute__((noinline)) int map_allocate(map_keys_equality *keq,
                                           map_key_hash *khash, int capacity,
                                           struct Map **map_out) {
  /* Tracing function and necessary variable(s) */
  klee_trace_ret();
  /* Necessary for map contract dependencies? */
  klee_trace_param_fptr(keq, "keq");
  klee_trace_param_fptr(khash, "khash");
  /* To differentiate between different maps */
  klee_trace_param_ptr(map_out, sizeof(struct Map *), "map_out");

  int allocation_succeeded = klee_int("map_allocation_succeeded");
  if (allocation_succeeded) {
    *map_out = malloc(sizeof(struct Map));
    klee_make_symbolic((*map_out), sizeof(struct Map), "map");
    klee_assert((*map_out) != NULL);

    memset((*map_out)->keys_present, 0, sizeof((*map_out)->keys_present));
    for (int n = 0; n < NUM_ELEMS; ++n) {
      (*map_out)->allocated_index[n] = 0;
      (*map_out)->key_deleted[n] = 0;
      // To speed up symbex when prototyping stuff unrelated to exec cycles,
      // make caching concrete
      // (*map_out)->keys_cached[n] = klee_int("map_keys_cached");
      (*map_out)->keys_cached[n] = 0;
    }
    (*map_out)->keys_seen = 0;

    (*map_out)->capacity = capacity;
    (*map_out)->ent_cond = 0;
    (*map_out)->keq = keq;
    (*map_out)->khash = khash;

    (*map_out)->Num_bucket_traversals = klee_int("Num_bucket_traversals");
    (*map_out)->Num_hash_collisions = klee_int("Num_hash_collisions");
    (*map_out)->occupancy = klee_range(0, capacity, "map_occupancy");

    /* For tracing map_key_cached */

    final_key_cached =
        malloc(strlen(base_key_cached) + strlen(suffix_key_cached) +
               2); /* Final string of the form - map_key_cached_occurence1 */
    klee_assert(final_key_cached && "Failed malloc");
    strcpy(final_key_cached, base_key_cached);
    strcpy(&final_key_cached[strlen(base_key_cached)], suffix_key_cached);

    return 1;
  }
  return 0;
}

void map_reset(struct Map *map) {
  // Do not trace. This function is an internal knob of the model.
  for (int n = 0; n < NUM_ELEMS; ++n) {
    map->allocated_index[n] = 0;
    map->key_deleted[n] = 0;
    // To speed up symbex when prototyping stuff unrelated to exec cycles, make
    // caching concrete map->keys_cached[n] = klee_int("map_keys_cached");
    map->keys_cached[n] = 0;
  }
  map->keys_seen = 0;
  map->occupancy = klee_range(0, map->capacity, "map_occupancy");
}

void map_set_key_size(struct Map *map, int size) { map->key_size = size; }

void map_set_id(struct Map *map, char *id, char *data_type) {
  map->id = malloc(strlen(id) + 1);
  strcpy(map->id, id);
  map->data_type = malloc(strlen(data_type) + 1);
  strcpy(map->data_type, data_type);
}

void map_set_entry_condition(struct Map *map, map_entry_condition *cond) {
  map->ent_cond = cond;
}

__attribute__((noinline)) int map_get(struct Map *map, void *key,
                                      int *value_out) {
  /* Tracing function and necessary variable(s) */
  klee_trace_ret();
  /* To differentiate between different maps */
  klee_trace_param_i32((uint32_t)map, "map");
  // klee_trace_param_tagged_ptr(key, map->key_size, "key", "", TD_BOTH);

  TRACE_VAL((uint32_t)(map), "map", _u32)
  TRACE_VAR(map->capacity, "map_capacity")
  TRACE_VAR(map->occupancy, "map_occupancy")
  TRACE_VAR(map->Num_bucket_traversals, "Num_bucket_traversals")
  TRACE_VAR(map->Num_hash_collisions, "Num_hash_collisions")
  TRACE_FPTR(map->khash, "map_hash")
  TRACE_FPTR(map->keq, "map_key_eq")

  /* For tracing map_has_this_key */
  int map_has_this_key;
  char *base_has_this_key = "map_has_this_key";
  char *suffix_has_this_key = "_occurence";
  char *final_has_this_key =
      malloc(strlen(base_has_this_key) + strlen(suffix_has_this_key) +
             2); /* Final string of the form - map_has_this_key_occurence1 */
  klee_assert(final_has_this_key && "Failed malloc");
  strcpy(final_has_this_key, base_has_this_key);
  strcpy(&final_has_this_key[strlen(base_has_this_key)], suffix_has_this_key);
  for (int n = 0; n < map->keys_seen; ++n) {
    void *key_ptr = map->keys_present + n * map->key_size;
    if (map->keq(key, key_ptr)) {
      if (map->key_deleted[n]) {
        map_has_this_key = 0;
        ds_path_1();
      } else {
        *value_out = map->allocated_index[n];
        map_has_this_key = 1;
        ds_path_2();
      }
      final_has_this_key[strlen(base_has_this_key) +
                         strlen(suffix_has_this_key)] = n + '0';
      final_has_this_key[strlen(base_has_this_key) +
                         strlen(suffix_has_this_key) + 1] = '\0';
      TRACE_VAR(map_has_this_key, final_has_this_key)

      final_key_cached[strlen(base_key_cached) + strlen(suffix_key_cached)] =
          n + '0';
      final_key_cached[strlen(base_key_cached) + strlen(suffix_key_cached) +
                       1] = '\0';
      TRACE_VAR(map->keys_cached[n], final_key_cached)
      if (!(map->keys_cached[n])) { /* Branching for Symbex */
        map->keys_cached[n] = 1;
      }

      return map_has_this_key;
    }
  }
  klee_assert(map->keys_seen < NUM_ELEMS && "No space left in the map stub");

  char *sym_name = "_in_";
  char *final_sym_name = (char *)malloc(1 + strlen(map->data_type) +
                                        strlen(sym_name) + strlen(map->id));
  strcpy(final_sym_name, map->data_type);
  strcat(final_sym_name, sym_name);
  strcat(final_sym_name, map->id);
  map_has_this_key = klee_int(final_sym_name);
  final_has_this_key[strlen(base_has_this_key) + strlen(suffix_has_this_key)] =
      map->keys_seen + '0';
  final_has_this_key[strlen(base_has_this_key) + strlen(suffix_has_this_key) +
                     1] = '\0';
  TRACE_VAR(map_has_this_key, final_has_this_key)

  final_key_cached[strlen(base_key_cached) + strlen(suffix_key_cached)] =
      map->keys_seen + '0';
  final_key_cached[strlen(base_key_cached) + strlen(suffix_key_cached) + 1] =
      '\0';
  TRACE_VAR(map->keys_cached[map->keys_seen], final_key_cached)

  if (!(map->keys_cached[map->keys_seen])) { /* Branching for Symbex */
    map->keys_cached[map->keys_seen] = 1;
    // VIGOR_TAG(TRAFFIC_CLASS, UNCACHED_FLOW);
    /* Only tag when adding a flow. From then on, it stays cached */
  } else {
    // VIGOR_TAG(TRAFFIC_CLASS, CACHED_FLOW);
  }

  // We might be reusing the symbol. This can lead to inadequate perf
  // descriptions
  klee_map_symbol_names(final_sym_name, map->keys_seen, key, map->key_size);

  void *key_ptr = map->keys_present + map->keys_seen * map->key_size;
  memcpy(key_ptr, key, map->key_size);
  map->allocated_index[map->keys_seen] = klee_int("allocated_index");
  if (map->ent_cond) {
    klee_assume(map->ent_cond(key_ptr, map->allocated_index[map->keys_seen]));
  }

  if (map_has_this_key) {
    map->key_deleted[map->keys_seen] = 0;
    *value_out = map->allocated_index[map->keys_seen];
    ds_path_2();
    map->keys_seen++;
    return 1;
  } else {
    map->key_deleted[map->keys_seen] = 1;
    ds_path_1();
    map->keys_seen++;
    return 0;
  }
}

__attribute__((noinline)) void map_put(struct Map *map, void *key, int value) {
  /* Tracing function and necessary variable(s) */
  klee_trace_ret();
  ds_path_1();
  /* To differentiate between different maps */
  klee_trace_param_i32((uint32_t)map, "map");

  TRACE_VAL((uint32_t)(map), "map", _u32)
  TRACE_VAR(map->capacity, "map_capacity")
  TRACE_VAR(map->occupancy, "map_occupancy")
  TRACE_VAR(map->Num_bucket_traversals, "Num_bucket_traversals")
  TRACE_VAR(map->Num_hash_collisions, "Num_hash_collisions")
  TRACE_FPTR(map->khash, "map_hash")

  if (map->ent_cond) {
    klee_assert(map->ent_cond(key, value));
  }
  map->occupancy += 1;
  for (int n = 0; n < map->keys_seen; ++n) {
    void *key_ptr = map->keys_present + n * map->key_size;
    if (map->keq(key, key_ptr)) {
      klee_assert(map->key_deleted[n] &&
                  "Trying to insert already present key");
      map->key_deleted[n] = 0;
      map->allocated_index[n] = value;
      if (!(map->keys_cached[n])) { /* Branching for Symbex */
        map->keys_cached[n] = 1;
      }
      final_key_cached[strlen(base_key_cached) + strlen(suffix_key_cached)] =
          n + '0';
      final_key_cached[strlen(base_key_cached) + strlen(suffix_key_cached) +
                       1] = '\0';
      TRACE_VAR(map->keys_cached[n], final_key_cached)
      return;
    }
  }
  klee_assert(map->keys_seen < NUM_ELEMS && "No space left in the map stub");
  void *key_ptr = map->keys_present + map->keys_seen * map->key_size;
  memcpy(key_ptr, key, map->key_size);
  map->allocated_index[map->keys_seen] = value;
  map->key_deleted[map->keys_seen] = 0;

  final_key_cached[strlen(base_key_cached) + strlen(suffix_key_cached)] =
      map->keys_seen + '0';
  final_key_cached[strlen(base_key_cached) + strlen(suffix_key_cached) + 1] =
      '\0';
  TRACE_VAR(map->keys_cached[map->keys_seen], final_key_cached)
  if (!(map->keys_cached[map->keys_seen])) { /* Branching for Symbex */
    map->keys_cached[map->keys_seen] = 1;
    // VIGOR_TAG(TRAFFIC_CLASS, UNCACHED_FLOW);
    /* Only tag when adding a flow. From then on, it stays cached */
  } else {
    // VIGOR_TAG(TRAFFIC_CLASS, CACHED_FLOW);
  }

  map->keys_seen++;
}

__attribute__((noinline)) void map_erase(struct Map *map, void *key,
                                         void **trash) {
  /* Tracing function and necessary variable(s) */
  klee_trace_ret();
  ds_path_1();
  /* To differentiate between different maps */
  klee_trace_param_i32((uint32_t)map, "map");

  TRACE_VAL((uint32_t)(map), "map", _u32)
  TRACE_VAR(map->capacity, "map_capacity")
  TRACE_VAR(map->occupancy, "map_occupancy")
  TRACE_VAR(map->Num_bucket_traversals, "Num_bucket_traversals")
  TRACE_VAR(map->Num_hash_collisions, "Num_hash_collisions")
  TRACE_FPTR(map->khash, "map_hash")
  TRACE_FPTR(map->keq, "map_key_eq")

  for (int n = 0; n < map->keys_seen; ++n) {
    void *key_ptr = map->keys_present + n * map->key_size;
    if (map->keq(key, key_ptr)) {
      map->occupancy -= 1;

      // It is important to differentiate the case
      // when that the key was deleted from the map,
      // as opposed to never existed on the first place.
      map->key_deleted[n] = 1;
      if (!(map->keys_cached[n])) { /* Branching for Symbex */
        map->keys_cached[n] = 1;
      }
      final_key_cached[strlen(base_key_cached) + strlen(suffix_key_cached)] =
          n + '0';
      final_key_cached[strlen(base_key_cached) + strlen(suffix_key_cached) +
                       1] = '\0';
      TRACE_VAR(map->keys_cached[n], final_key_cached)
      return; // The key is deleted, job's done
    }
  }
  klee_assert(map->keys_seen < NUM_ELEMS && "No space left in the map stub");
  // The key was not previously mentioned,
  // but we need to take a note that the key was deleted,
  // in case we access it in the future.
  void *key_ptr = map->keys_present + map->keys_seen * map->key_size;
  memcpy(key_ptr, key, map->key_size);
  map->key_deleted[map->keys_seen] = 1;

  final_key_cached[strlen(base_key_cached) + strlen(suffix_key_cached)] =
      map->keys_seen + '0';
  final_key_cached[strlen(base_key_cached) + strlen(suffix_key_cached) + 1] =
      '\0';
  TRACE_VAR(map->keys_cached[map->keys_seen], final_key_cached)
  if (!(map->keys_cached[map->keys_seen])) { /* Branching for Symbex */
    map->keys_cached[map->keys_seen] = 1;
    // VIGOR_TAG(TRAFFIC_CLASS, UNCACHED_FLOW);
    /* Only tag when adding a flow. From then on, it stays cached */
  } else {
    // VIGOR_TAG(TRAFFIC_CLASS, CACHED_FLOW);
  }
  map->keys_seen++;
}

__attribute__((noinline)) int map_size(struct Map *map) {
  klee_trace_ret();
  ds_path_1();
  /* To differentiate between different maps */
  klee_trace_param_i32((uint32_t)map, "map");
  char *sym_name = ".size";
  char *final_sym_name = (char *)malloc(1 + strlen(map->id) + strlen(sym_name));
  strcpy(final_sym_name, map->id);
  strcat(final_sym_name, sym_name);
  return klee_int(final_sym_name);
}
