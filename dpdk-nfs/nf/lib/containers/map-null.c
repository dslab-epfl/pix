#include "lib/containers/map.h"

#include <stdlib.h>

struct Map { };

static int nullmap_size;

int
map_allocate(map_keys_equality* keq, map_key_hash* khash, int capacity, struct Map** map_out)
{
	*map_out = malloc(sizeof(struct Map));
	if (*map_out == NULL) {
		return 0;
	}

	return 1;
}

void
map_put(struct Map* map, void* key, int value)
{
	nullmap_size++;
}

void
map_erase(struct Map* map, void* key, void** trash)
{
	nullmap_size--;
}

int
map_get(struct Map* map, void* key, int* value_out)
{
	if (nullmap_size == 0) {
		return 0;
	}

	*value_out = 0;
	return 1;
}
