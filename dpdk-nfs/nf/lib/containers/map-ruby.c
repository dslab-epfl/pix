#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "unverified-nat/ruby/config.h"
#include "unverified-nat/ruby/st.h" // Modified so that st_data_t == __uint128_t

#include "lib/flow.h"
#include "lib/containers/map.h"


struct Map {
	st_table* value;
};

static map_key_hash* map_hash_fn;
static map_keys_equality* map_eq_fn;

static struct st_hash_type ruby_hashcmp;

static int
ruby_comparer(st_data_t left, st_data_t right)
{
	return (*map_eq_fn)(&left, &right);
}

static st_index_t
ruby_hasher(st_data_t value)
{
	return (*map_hash_fn)(&value);
}


int
map_allocate(map_keys_equality* keq, map_key_hash* khash, int capacity, struct Map** map_out)
{
	*map_out = (struct Map*) malloc(sizeof(struct Map));
	if (*map_out == NULL) {
		return 0;
	}

	map_hash_fn = khash;
	map_eq_fn = keq;
	ruby_hashcmp.compare = &ruby_comparer;
	ruby_hashcmp.hash = &ruby_hasher;
	// I checked the source, st_init_table just ignores malloc errors...
	(*map_out)->value = st_init_table_with_size(&ruby_hashcmp, (st_index_t) capacity);
	return 1;
}

void
map_put(struct Map* map, void* key, int value)
{
	st_insert(map->value, *((st_data_t*) key), (st_data_t) value);
}

void
map_erase(struct Map* map, void* key, void** trash)
{
	st_data_t st_trash;
	st_delete(map->value, (st_data_t*) key, &st_trash);
	*trash = NULL; // I give up...
}

int
map_get(struct Map* map, void* key, int* value_out)
{
	st_data_t st_value;
	if (st_lookup(map->value, *((st_data_t*) key), &st_value)) {
		*value_out = (int) st_value;
		return 1;
	}
	return 0;
}
