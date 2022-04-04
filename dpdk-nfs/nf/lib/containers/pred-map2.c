#include <stdlib.h>
#include "pred-map.h"

#ifdef DUMP_PERF_VARS
#include "lib/nf_log.h"
#endif

struct Map
{
  struct Bucket *buckets;
  int capacity; //Must be a power of 2
  int size;
  map_keys_equality *keys_eq;
  map_key_hash *khash;
};

#ifndef NULL
#define NULL 0
#endif //NULL

int map_allocate(map_keys_equality *keq, map_key_hash *khash,
                 int capacity,
                 struct Map **map_out)
{
  struct Map *old_map_val = *map_out;
  struct Map *map_alloc = malloc(sizeof(struct Map));
  if (map_alloc == NULL)
    return 0;
  *map_out = (struct Map *)map_alloc;

  struct Bucket *buckets_alloc = malloc(sizeof(*buckets_alloc) * capacity);
  if (buckets_alloc == NULL)
  {
    free(map_alloc);
    *map_out = old_map_val;
    return 0;
  }
  (*map_out)->buckets = buckets_alloc;
  (*map_out)->capacity = capacity;
  (*map_out)->size = 0;
  (*map_out)->keys_eq = keq;
  (*map_out)->khash = khash;

  map_impl_init((*map_out)->buckets,
                keq,
                capacity);
  return 1;
}

int map_get_1(struct Map *map, void *key, int *value_out)
{
#ifdef DUMP_PERF_VARS
  perf_dump_prefix = "map_";
  perf_dump_suffix = "";
#endif
  map_key_hash *khash = map->khash;
  int hash = khash(key);
  return map_impl_get(map->buckets,
                      key,
                      map->keys_eq,
                      hash,
                      value_out,
                      map->capacity);
}

int map_get_2(struct Map *map, void *key, int *value_out)
{
#ifdef DUMP_PERF_VARS
  perf_dump_prefix = "map_";
  perf_dump_suffix = "";
#endif
  map_key_hash *khash = map->khash;
  int hash = khash(key);
  return map_impl_get(map->buckets,
                      key,
                      map->keys_eq,
                      hash,
                      value_out,
                      map->capacity);
}

void map_put(struct Map *map, void *key, int value)
{
#ifdef DUMP_PERF_VARS
  perf_dump_prefix = "map_";
  perf_dump_suffix = "";
#endif
  map_key_hash *khash = map->khash;
  int hash = khash(key);
  map_impl_put(map->buckets,
               key, hash, value,
               map->capacity);
  ++map->size;
}

void map_erase(struct Map *map, void *key, void **trash)
{
#ifdef DUMP_PERF_VARS
  perf_dump_prefix = "map_";
  perf_dump_suffix = "";
#endif

  map_key_hash *khash = map->khash;
  int hash = khash(key);
  map_impl_erase(map->buckets,
                 key,
                 map->keys_eq,
                 hash,
                 map->capacity,
                 trash);
  --map->size;
}

int map_size(struct Map *map)
{
  return map->size;
}
