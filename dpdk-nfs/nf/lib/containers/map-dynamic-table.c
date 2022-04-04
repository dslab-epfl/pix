#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "lib/flow.h"
#include "lib/containers/map.h"

// Map using DPDK table.
// For keys we use int_key, but it'll work with ext_key as well since they're of
// the same size

#define NUM_COLLISIONS_FOR_RESIZE 1

struct Mapping {
  void *key;
  int value;
  struct Mapping *next;
};

struct Map {
  int size;
  struct Mapping **table;

  map_keys_equality *keq;
  map_key_hash *khash;
};

int map_allocate(map_keys_equality *keq, map_key_hash *khash, int capacity,
                 struct Map **map_out) {
  if (!(keq && khash && capacity > 0 && map_out)) {
    return 0;
  }


  *map_out = malloc(sizeof(struct Map));
  if (!*map_out) {
    return 0;
  }

  (*map_out)->size = capacity;
  (*map_out)->keq = keq;
  (*map_out)->khash = khash;

  (*map_out)->table = calloc(capacity, sizeof(void *));
  if (!(*map_out)->table) {
    free(*map_out);
    *map_out = NULL;
    return 0;
  }

  return 1;
}

int map_put_internal(struct Map *map, void *key, int value) {
  int bucket = map->khash(key) & ( map->size -1 );

  int num_collisions = 0;
  for (struct Mapping *mapping = map->table[bucket]; mapping;
       mapping = mapping->next, num_collisions++) {
    if (map->keq(key, mapping->key)) {
      mapping->value = value;
      return num_collisions;
    }
  }

  struct Mapping *mapping = malloc(sizeof(struct Mapping));
  assert(mapping);
  mapping->key = key;
  mapping->value = value;
  mapping->next = map->table[bucket];
  map->table[bucket] = mapping;

  return num_collisions;
}

void map_embiggen(struct Map *map) {
  struct Map *new_map;
  int result = map_allocate(map->keq, map->khash, map->size * 2 , &new_map);
  assert(result && map);

  for (int i = 0; i < map->size; i++) {
    for (struct Mapping *mapping = map->table[i]; mapping;
         mapping = map->table[i]) {
      map_put_internal(new_map, mapping->key, mapping->value);
      map_erase(map, mapping->key, NULL);
    }
  }

  *map = *new_map;
  free(new_map);
}

void map_put(struct Map *map, void *key, int value) {
  assert(map && map->table && key);

  if (map_put_internal(map, key, value) >= NUM_COLLISIONS_FOR_RESIZE) {
    map_embiggen(map);
  }
}

void map_erase(struct Map *map, void *key, void **trash) {
  assert(map && map->table && key);

  int bucket = map->khash(key)& ( map->size -1 );

  for (struct Mapping **mapping = &map->table[bucket]; *mapping;
       mapping = &(*mapping)->next) {
    if (map->keq(key, (*mapping)->key)) {
      if (trash) {
        *trash = (*mapping)->key;
      }
      struct Mapping *old_mapping = *mapping;
      *mapping = (*mapping)->next;
      free(old_mapping);
      break;
    }
  }
}

int map_get(struct Map *map, void *key, int *value_out) {
  assert(map && map->table && key && value_out);

  int bucket = map->khash(key) &( map->size-1);

  for (struct Mapping *mapping = map->table[bucket]; mapping;
       mapping = mapping->next) {
    if (map->keq(key, mapping->key)) {
      *value_out = mapping->value;
      return 1;
    }
  }

  return 0;
}

// bool eq(void *a, void *b) { return *((int *)a) == *((int *)b); }
// 
// int hash(void *k) { return *((int *)k); }
// 
// int main(int argc, char *argv[]) {
//   struct Map *map;
//   int result = map_allocate(eq, hash, 1, &map);
//   assert(result && map);
// 
//   for (int i = 0; i < 10; i++) {
//     int *key = (int *)malloc(sizeof(struct int_key));
//     *key = i;
// 
//     map_put(map, key, i);
//     printf("PUT %d - size=%d\n", i, map->size);
//   }
// 
//   for (int i = 0; i < 10; i++) {
//     int *key = malloc(sizeof(struct int_key));
//     *key = i;
// 
//     int value;
//     result = map_get(map, key, &value);
//     assert(result);
//     printf("GET %d = %d\n", i, value);
//   }
// 
//   for (int i = 0; i < 10; i++) {
//     int *key = malloc(sizeof(struct int_key));
//     *key = i;
// 
//     printf("ERASE %d\n", i);
//     void *trash;
//     map_erase(map, key, &trash);
//     free(trash);
//   }
// 
//   for (int i = 0; i < 10; i++) {
//     int *key = malloc(sizeof(struct int_key));
//     *key = i;
// 
//     printf("GET %d\n", i);
//     int value;
//     result = map_get(map, key, &value);
//     assert(!result);
//   }
// 
//   return 0;
// }
