#ifndef _DOUBLE_MAP_H_INCLUDED_
#define _DOUBLE_MAP_H_INCLUDED_

#include "pred-map-impl.h"
#include "map-util.h"
#include <stdint.h>

/*
  This implementation expects keys to be the part of the value. The keys
  are extracted with dmap_extract_keys function and are put back with
  dmap_pack_keys.
 */

typedef void uq_value_copy(char *dst, void *src);

typedef void dmap_extract_keys(void *vp, void **kpp1, void **kpp2);

typedef void dmap_pack_keys(void *vp, void *kp1, void *kp2);

typedef void uq_value_destr(void *vp);

struct DoubleMap;

/**
   Allocate and initialize a new hash table. The extensive argument list defines
   the entry and two keys(A and B) to access it.

   @param eq_a - the equality function for the key A.
   @param hsh_a - the hash function for the key A.
   @param eq_b - the equality function for the key B.
   @param hsh_b - the hash function for the key B.
   @param value_size - the size of the value entry.
   @param v_cpy - the copy function, allowing to duplicate the value at the
                  given preallocated memory chunk.
   @param v_destr - the destroy function for the value entry. Frees all the
                    allocated resources for the entry, except the memory chunk.
   @param dexk - the extract function, allowing to get pointers to keys A and B
                 from the given value entry.
   @param dpk - the reverse function, packing the extracted keys back to the
                value.
   @param capacity - the table capacity - maximum number of entries stored
                     simultaneously.
   @param map_out - output pointer to the allocated map
   @returns 1 if the map is successfully allocated, 0 otherwise.
 */
int dmap_allocate(map_keys_equality *eq_a, map_key_hash *hsh_a,
                  map_keys_equality *eq_b, map_key_hash *hsh_b,
                  int value_size, uq_value_copy *v_cpy,
                  uq_value_destr *v_destr,
                  dmap_extract_keys *dexk,
                  dmap_pack_keys *dpk,
                  int capacity,
                  struct DoubleMap **map_out);

/**
   Get the internal index by key A. See dmap_get_value to obtain the full entry.

   @param map - pointer to the hash table.
   @param key - pointer to the key A.
   @param index - output pointer to the index of the entry by the key A.
   @returns 1 if the entry is found, 0 otherwise.
 */
int dmap_get_a /*@ <K1,K2,V> @*/ (struct DoubleMap *map, void *key, int *index);

/**
   Get the internal index by key B. See dmap_get_value to obtain the full entry.

   @param map - pointer to the hash table.
   @param key - pointer to the key B.
   @param index - output pointer to the index of the entry by the key B.
   @returns 1 if the entry is found, 0 otherwise.
*/
int dmap_get_b(struct DoubleMap *map, void *key, int *index);

/**
   Add entry to the map. Use the internal index 'index'. The index must be
   uinque for this map for this moment, use an allocator to ensure that (See
   double-chain.h).

   @param map - pointer to the hash table.
   @param value - pointer to the value being added.
   @param index - a unique index, 0 <= index < map_capacity.
   @returns 1.
 */
int dmap_put(struct DoubleMap *map, void *value, int index);

/**
   Copy the value by the index into the preallocated memory chunk. The index
   must be returned by dmap_get_{a,b}, or just used in dmap_put.

   @param map - pointer to the hash table.
   @param index - the internal index of the value.
   @param valut_out - the preallocated memory chunk, to hold the copy of the
                      value.
 */
void dmap_get_value(struct DoubleMap *map, int index,
                    void *value_out);

/**
   Remove an entry from the hash table. The index must point to an existing
   entry.

   @param map - pointer to the hash table.
   @param index - the internal index, known to be used by the map.
   @returns 1.
 */
int dmap_erase(struct DoubleMap *map, int index);
/**
   Get the number of entries in a hash table.

   @param map - pointer to the hash table.
   @returns the number of entries in the table.
 */
int dmap_size(struct DoubleMap *map);

#endif // _DOUBLE_MAP_H_INCLUDED_
