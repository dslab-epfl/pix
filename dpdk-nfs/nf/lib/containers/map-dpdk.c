#include <rte_common.h>
#include <rte_mbuf.h>
#include <rte_table.h>
#include <rte_table_hash.h>

#include "lib/flow.h"
#include "lib/containers/map.h"

// Map using DPDK table.
// For keys we use int_key, but it'll work with ext_key as well since they're of the same size

// DPDK's "table" structure is meant to use packets, i.e. rte_mbufs, as keys.
// However, it never actually accesses the packet-specific things,
// they're opaque (modulo an user-configured offset, which we force to be 0).
// Since this is C, well... a pointer is a pointer is a pointer.


struct Map {
	void* value;
};

static map_key_hash* map_hash_fn;

static uint64_t
nat_map_hash_fn_dpdk(void* key, void* key_mask, uint32_t key_size, uint64_t seed)
{
	return (*map_hash_fn)(key);
}


int
map_allocate(map_keys_equality* keq, map_key_hash* khash, int capacity, struct Map** map_out)
{
	// hacky hack, we know capacity is 1 less than a power of 2
	capacity++;

	struct rte_table_hash_params table_params;
	table_params.name = "table";
	table_params.key_size = sizeof(struct int_key);
	table_params.key_offset = 0; // MUST be 0, see remark at top of file
	table_params.key_mask = NULL; // unused
	table_params.n_keys = capacity;
	table_params.n_buckets = capacity >> 2;
	table_params.f_hash = &nat_map_hash_fn_dpdk;
	table_params.seed = 0; // unused

	map_hash_fn = khash;

	// 2nd param is socket ID, we don't really need it
	void* dpdk_table = rte_table_hash_ext_ops.f_create(&table_params, 0, sizeof(int));
	if (dpdk_table == NULL) {
		rte_exit(EXIT_FAILURE, "Out of memory in map_allocate for rte_table\n");
	}

	*map_out = (struct Map*) malloc(sizeof(struct Map));
	if (*map_out == NULL) {
		rte_exit(EXIT_FAILURE, "Out of memory in map_allocate for nat_map\n");
	}

	(*map_out)->value = dpdk_table;
	return 1;
}

void
map_put(struct Map* map, void* key, int value)
{
	// The add function allows to both check if the value was already there, and get a handle to the entry.
	// We care about neither.
	int unused_key_found;
	void* unused_entry_ptr;

	int ret = rte_table_hash_ext_ops.f_add(map->value, key, &value, &unused_key_found, &unused_entry_ptr);
	if (ret != 0) {
		rte_exit(ret, "Error in map_put\n");
	}
}

void
map_erase(struct Map* map, void* key, void** trash)
{
	// Same remark as insert
	int unused_key_found;

	int ret = rte_table_hash_ext_ops.f_delete(map->value, key, &unused_key_found, trash);
	if (ret != 0) {
		rte_exit(ret, "Error in map_erase\n");
	}
}

int
map_get(struct Map* map, void* key, int* value_out)
{
	uint64_t lookup_hit_mask;
	// rte_table requires values to be a fully valid 64-entry array
	void* values[64];

	int ret = rte_table_hash_ext_ops.f_lookup(
		map->value,
		(struct rte_mbuf**) &key, // keys: pseudo-array of pseudo-mbufs
		RTE_LEN2MASK(1, uint64_t), // bitmask of valid keys
		&lookup_hit_mask,
		values
	);
	if (ret != 0) {
		rte_exit(ret, "Error in map_get\n");
	}

	if (lookup_hit_mask == 0) {
		return 0;
	}

	*value_out = *((int*) values[0]);
	return 1;
}
