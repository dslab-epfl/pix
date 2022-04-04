#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "pred-double-map.h"
#include <stdio.h>

#ifdef DUMP_PERF_VARS
#include "lib/nf_log.h"
#endif

//@ #include "arith.gh"

struct DoubleMap
{
  int value_size;

  uq_value_copy *cpy;
  uq_value_destr *dstr;

  uint8_t *values;

  struct Bucket *buckets_a;
  map_keys_equality *eq_a;
  map_key_hash *hsh_a;

  struct Bucket *buckets_b;
  map_keys_equality *eq_b;
  map_key_hash *hsh_b;

  dmap_extract_keys *exk;
  dmap_pack_keys *pk;

  int size;
  int capacity;
};

#ifndef NULL
#define NULL 0
#endif //NULL

int dmap_allocate(map_keys_equality *eq_a, map_key_hash *hsh_a,
                  map_keys_equality *eq_b, map_key_hash *hsh_b,
                  int value_size, uq_value_copy *v_cpy,
                  uq_value_destr *v_destr,
                  dmap_extract_keys *dexk,
                  dmap_pack_keys *dpk,
                  int capacity,
                  struct DoubleMap **map_out)
{
  struct DoubleMap *old_map_val = *map_out;
  struct DoubleMap *map_alloc = malloc(sizeof(struct DoubleMap));
  if (map_alloc == NULL)
    return 0;
  *map_out = (struct DoubleMap *)map_alloc;

  uint8_t *vals_alloc = malloc(value_size * capacity);
  if (vals_alloc == NULL)
  {
    free(map_alloc);
    *map_out = old_map_val;
    return 0;
  }
  (*map_out)->values = vals_alloc;

  struct Bucket *buckets_a_alloc = malloc(sizeof(*buckets_a_alloc) * capacity);
  if (buckets_a_alloc == NULL)
  {
    free(map_alloc);
    free(vals_alloc);
    *map_out = old_map_val;
    return 0;
  }
  (*map_out)->buckets_a = buckets_a_alloc;

  struct Bucket *buckets_b_alloc = malloc(sizeof(*buckets_b_alloc) * capacity);
  if (buckets_b_alloc == NULL)
  {
    free(map_alloc);
    free(vals_alloc);
    free(buckets_a_alloc);
    *map_out = old_map_val;
    return 0;
  }
  (*map_out)->buckets_b = buckets_b_alloc;

  (*map_out)->eq_a = eq_a;
  (*map_out)->hsh_a = hsh_a;
  (*map_out)->eq_b = eq_b;
  (*map_out)->hsh_b = hsh_b;
  (*map_out)->value_size = value_size;
  (*map_out)->cpy = v_cpy;
  (*map_out)->dstr = v_destr;
  (*map_out)->exk = dexk;
  (*map_out)->pk = dpk;
  (*map_out)->capacity = capacity;

  map_impl_init((*map_out)->buckets_a, (*map_out)->eq_a,
                (*map_out)->capacity);

  map_impl_init((*map_out)->buckets_b, (*map_out)->eq_b,
                (*map_out)->capacity);

  (*map_out)->size = 0;

  return 1;
}

int dmap_get_a(struct DoubleMap *map, void *key, int *index)
{
#ifdef DUMP_PERF_VARS
  perf_dump_prefix = "dmap_";
  perf_dump_suffix = "_a";
#endif

  map_key_hash *hsh_a = map->hsh_a;

  int hash = hsh_a(key);
  int rez = map_impl_get(map->buckets_a, key,
                         map->eq_a, hash, index,
                         map->capacity);

  return rez;
}

int dmap_get_b(struct DoubleMap *map, void *key, int *index)
{
#ifdef DUMP_PERF_VARS
  perf_dump_prefix = "dmap_";
  perf_dump_suffix = "_b";
#endif

  map_key_hash *hsh_b = map->hsh_b;

  int hash = hsh_b(key);

  return map_impl_get(map->buckets_b, key,
                      map->eq_b, hash, index,
                      map->capacity);
}

int dmap_put(struct DoubleMap *map, void *value, int index)
{
#ifdef DUMP_PERF_VARS
  perf_dump_prefix = "dmap_";
  perf_dump_suffix = "_a";
#endif

  void *key_a = 0;
  void *key_b = 0;

  void *my_value = map->values + index * map->value_size;
  uq_value_copy *cpy = map->cpy;
  cpy(my_value, value);

  dmap_extract_keys *exk = map->exk;
  exk(my_value, &key_a, &key_b);

  map_key_hash *hsh_a = map->hsh_a;
  int hash1 = hsh_a(key_a);

  map_impl_put(map->buckets_a, key_a,
               hash1, index, map->capacity);

#ifdef DUMP_PERF_VARS
  perf_dump_prefix = "dmap_";
  perf_dump_suffix = "_b";
#endif

  map_key_hash *hsh_b = map->hsh_b;
  int hash2 = hsh_b(key_b);

  map_impl_put(map->buckets_b, key_b,
               hash2, index, map->capacity);

  ++map->size;
  dmap_pack_keys *pk = map->pk;
  pk(my_value, key_a, key_b);

  return 1;
}

void dmap_get_value(struct DoubleMap *map, int index,
                    void *value_out)
{
  void *my_value = map->values + index * map->value_size;
  uq_value_copy *cpy = map->cpy;
  cpy(value_out, my_value);
}

int dmap_erase(struct DoubleMap *map, int index)
{
#ifdef DUMP_PERF_VARS
  perf_dump_prefix = "dmap_";
  perf_dump_suffix = "_a";
#endif

  void *key_a = 0;
  void *out_key_a = 0;
  void *key_b = 0;
  void *out_key_b = 0;

  void *my_value = map->values + index * map->value_size;
  dmap_extract_keys *exk = map->exk;
  exk(my_value, &key_a, &key_b);

  map_key_hash *hsh_a = map->hsh_a;
  int hash1 = hsh_a(key_a);

  map_impl_erase(map->buckets_a, key_a,
                 map->eq_a, hash1,
                 map->capacity, &out_key_a);

#ifdef DUMP_PERF_VARS
  perf_dump_prefix = "dmap_";
  perf_dump_suffix = "_b";
#endif

  map_key_hash *hsh_b = map->hsh_b;
  int hash2 = hsh_b(key_b);

  map_impl_erase(map->buckets_b, key_b,
                 map->eq_b, hash2,
                 map->capacity, &out_key_b);

  dmap_pack_keys *pk = map->pk;
  pk(my_value, key_a, key_b);
  pk(my_value, out_key_a, out_key_b);
  uq_value_destr *dstr = map->dstr;
  dstr(my_value);
  --map->size;

  return 1;
}

int dmap_size(struct DoubleMap *map)
{
  return map->size;
}
