#ifndef _MAP_H_INCLUDED_
#define _MAP_H_INCLUDED_

#include "pred-map-impl.h"
#include "map-util.h"

struct Map;

int map2_allocate(map_keys_equality *keq,
                  map_key_hash *khash, int capacity,
                  struct Map **map_out);

int map2_get_1(struct Map *map, void *key, int *value_out);

int map2_get_2(struct Map *map, void *key, int *value_out);

void map2_put(struct Map *map, void *key, int value);

void map2_erase(struct Map *map, void *key, void **trash);

int map2_size(struct Map *map);

#endif //_MAP_H_INCLUDED_
