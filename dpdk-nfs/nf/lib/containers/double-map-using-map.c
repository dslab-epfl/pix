#include "lib/containers/double-map.h"
#include "lib/containers/map.h"

#include <stdlib.h>


struct DoubleMap {
	struct Map* first;
	struct Map* second;

	int size;
	int capacity;
	int value_size;
	void** values;

	uq_value_copy* value_copy;
	uq_value_destr* value_destr;

	dmap_extract_keys* key_extract;
	dmap_pack_keys* key_pack;
};

int
dmap_allocate(map_keys_equality* eq_a,
		map_key_hash* hash_a,
		map_keys_equality* eq_b,
		map_key_hash* hash_b,
		int value_size,
		uq_value_copy* value_copy,
		uq_value_destr* value_destr,
		dmap_extract_keys* key_extract,
		dmap_pack_keys* key_pack,
		int capacity,
		int underlying_capacity,
		struct DoubleMap** map_out)
{
	// Create the first map
	struct Map* first = NULL;
	if (map_allocate(eq_a, hash_a, underlying_capacity, &first) == 0) {
		goto fail;
	}

	// Create the second map
	struct Map* second = NULL;
	if (map_allocate(eq_b, hash_b, underlying_capacity, &second) == 0) {
		goto fail;
	}

	// Pre-create the values storage to avoid heap allocs later
	void* values = calloc(underlying_capacity, value_size);
	if (values == NULL) {
		goto fail;
	}

	// Create the double-map
	struct DoubleMap* dmap = malloc(sizeof(struct DoubleMap));
	if (dmap == NULL) {
		goto fail;
	}

	// Initialize the double-map
	dmap->first = first;
	dmap->second = second;
	dmap->size = 0;
	dmap->capacity = capacity;
	dmap->value_size = value_size;
	dmap->values = values;
	dmap->value_copy = value_copy;
	dmap->value_destr = value_destr;
	dmap->key_extract = key_extract;
	dmap->key_pack = key_pack;

	*map_out = dmap;
	return 1;

fail:
	free(first);
	free(second);
	free(values);
	free(dmap);

	return 0;
}

int
dmap_get_a(struct DoubleMap* dmap, void* key, int* index)
{
	return map_get(dmap->first, key, index);
}

int
dmap_get_b(struct DoubleMap* dmap, void* key, int* index)
{
	return map_get(dmap->second, key, index);
}

void
dmap_get_value(struct DoubleMap* dmap, int index, void* value_out)
{
	// Copy the value (we don't give out our own memory!)
	dmap->value_copy(value_out, dmap->values[index]);
}

int
dmap_put(struct DoubleMap* dmap, void* value, int index)
{
	// Copy the value (we don't store originals!)
	void* value_copy = malloc(dmap->value_size);
	if (value_copy == NULL) {
		return 0;
	}
	dmap->value_copy(value_copy, value);

	// Insert the value
	dmap->values[index] = value_copy;

	// Get the keys
	void* a;
	void* b;
	dmap->key_extract(value_copy, &a, &b);

	// Add the keys to the maps
	map_put(dmap->first, a, index);
	map_put(dmap->second, b, index);

	// Update the size
	dmap->size = dmap->size + 1;

	return 1;
}

int
dmap_erase(struct DoubleMap* dmap, int index)
{
	// Get the value
	void* value = dmap->values[index];

	// Get the keys
	void* a;
	void* b;
	dmap->key_extract(value, &a, &b);

	// Get the old first key
	void* trash_a;
	map_erase(dmap->first, a, &trash_a);

	// Get the old second key
	void* trash_b;
	map_erase(dmap->second, b, &trash_b);

	// Pack trash together, delete it
	dmap->key_pack(value, trash_a, trash_b);
	free(value);

	// Update the size
	dmap->size = dmap->size - 1;

	return 1;
}

int
dmap_size(struct DoubleMap* dmap)
{
	return dmap->size;
}
