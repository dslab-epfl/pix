#include "lib/containers/map.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#define MAP_BUCKETS_POWER 16
#define MAP_INITIAL_FREE_LIST_CAP 4

struct MapNode;
struct MapNode {
	void* key;
	int value;
	struct MapNode* next;
};

struct Map {
	struct MapNode* buckets[1 << MAP_BUCKETS_POWER];
	struct MapNode* free_lists[1 << MAP_BUCKETS_POWER];
	int size;

	map_keys_equality* key_equality;
	map_key_hash* key_hash;
};


// === Helper functions ===

// Initializes a free list
void
map_init_free_list(struct MapNode* free_list)
{
	for (int n = 0; n < MAP_INITIAL_FREE_LIST_CAP - 1; n++) {
		free_list[n].next = &(free_list[n + 1]);
	}
}

// Gets the bucket that the specified key should use
int
map_get_bucket(struct Map* map, void* key)
{
	int hash = map->key_hash(key);
	return hash & ((1 << MAP_BUCKETS_POWER) - 1);
}

// Gets the closest non-empty free list for a bucket
struct MapNode**
map_get_free_list(struct Map* map, int bucket)
{
	int max = (1 << MAP_BUCKETS_POWER) - 1;
	for (int n = 0; n <= max; n++) {
		int cand_bucket = (bucket + n) & max;
		if (map->free_lists[cand_bucket] != NULL) {
			return &(map->free_lists[cand_bucket]);
		}
	}

	// This should never happen since there are more free nodes than map capacity
	abort();
	return NULL;
}

// Finds the node storing a key equivalent to the specified one, and also returns the previous node; or return false if it doesn't exist
bool
map_get_node(struct Map* map, void* key, struct MapNode** out_node, struct MapNode** out_prev)
{
	// Get the bucket
	int bucket = map_get_bucket(map, key);

	// Iterate all nodes til we find one with an equivalent key, keeping track of the previous node
	*out_node = map->buckets[bucket];
	*out_prev = NULL;
	while (*out_node != NULL) {
		if (map->key_equality(key, (*out_node)->key)) {
			return true;
		}

		*out_prev = *out_node;
		*out_node = (*out_node)->next;
	}

	return false;
}


// === Interface implementation ===

int
map_allocate(map_keys_equality* key_equality, map_key_hash* key_hash, int capacity, struct Map** map_out)
{
	// Create the map (calloc to zero-init)
	struct Map* map = calloc(1, sizeof(struct Map));
	if (map == NULL) {
		goto fail;
	}

	// Create and initialize the free lists
	for (int n = 0; n < sizeof(map->free_lists)/sizeof(map->free_lists[0]); n++) {
		map->free_lists[n] = calloc(MAP_INITIAL_FREE_LIST_CAP, sizeof(struct MapNode));
		if (map->free_lists[n] == NULL) {
			goto fail;
		}
		map_init_free_list(map->free_lists[n]);
	}

	// Initialize the map
	map->size = 0;
	map->key_equality = key_equality;
	map->key_hash = key_hash;

	*map_out = map;

	return 1;

fail:
	if (map != NULL) {
		for (int n = 0; n < sizeof(map->free_lists)/sizeof(map->free_lists[0]); n++) {
			free(map->free_lists[n]);
		}
	}
	free(map);

	return 0;
}

void
map_put(struct Map* map, void* key, int value)
{
	// Get the bucket
	int bucket = map_get_bucket(map, key);

	// Get a free list
	struct MapNode** free_list = map_get_free_list(map, bucket);

	// Get a node
	struct MapNode* node = *free_list;
	*free_list = node->next;

	// Initialize the node
	node->key = key;
	node->value = value;

	// Add the node to the bucket
	node->next = map->buckets[bucket];
	map->buckets[bucket] = node;

	// Update the size
	map->size = map->size + 1;
}

void
map_erase(struct Map* map, void* key, void** trash)
{
	// Get the bucket
	int bucket = map_get_bucket(map, key);

	// Get the node and its predecessor, we know it exists
	struct MapNode* node;
	struct MapNode* prev;
	map_get_node(map, key, &node, &prev);

	// Remove the node from the bucket's list;
	// If there is no predecessor, use the bucket
	if (prev == NULL) {
		map->buckets[bucket] = node->next;
	} else {
		prev->next = node->next;
	}

	// Get a free list
	struct MapNode** free_list = map_get_free_list(map, bucket);

	// Add the node to the free list
	node->next = *free_list;
	*free_list = node;

	// Update the size
	map->size = map->size - 1;

	// Return original key
	*trash = node->key;
}

int
map_get(struct Map* map, void* key, int* value_out)
{
	// Get the node, ignore the predecessor
	struct MapNode* node;
	struct MapNode* _;
	if (map_get_node(map, key, &node, &_)) {
		// Return the value
		*value_out = node->value;

		return 1;
	}

	return 0;
}

int
map_size(struct Map* map)
{
	return map->size;
}
