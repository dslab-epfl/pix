#include <stdlib.h>
#include "lib/containers/rehashing-map-impl.h"
#include "map.h"

#ifdef DUMP_PERF_VARS
#include "lib/nf_log.h"
#endif

#define REHASHING_THRESHOLD 5

struct Map_Internal {
  int* busybits;
  void** keyps;
  int* khs;
  int* chns;
  int* vals;
  int capacity;
  int size;
  map_keys_equality* keys_eq;
  map_key_hash* khash;
  int hash_seed;
};

struct Map {
  struct Map_Internal *maps[2];
  int current_map;
};
#define CURRENT_MAP(ptr) ((ptr)->maps[(ptr)->current_map])
#define OTHER_MAP(ptr) ((ptr)->maps[!((ptr)->current_map)])

#ifndef NULL
#define NULL 0
#endif//NULL

/*@
  predicate mapp<t>(struct Map_Internal* ptr,
                    predicate (void*;t) kp,
                    fixpoint (t,int) hsh,
                    fixpoint (t,int,bool) recp,
                    mapi<t> map) =
    malloc_block_Map(ptr) &*&
    ptr->busybits |-> ?busybits &*&
    ptr->keyps |-> ?keyps &*&
    ptr->khs |-> ?khs &*&
    ptr->chns |-> ?chns &*&
    ptr->vals |-> ?vals &*&
    ptr->capacity |-> ?capacity &*&
    ptr->size |-> ?size &*&
    ptr->keys_eq |-> ?keys_eq &*&
    ptr->khash |-> ?khash &*&
    malloc_block_ints(busybits, capacity) &*&
    malloc_block_pointers(keyps, capacity) &*&
    malloc_block_ints(khs, capacity) &*&
    malloc_block_ints(chns, capacity) &*&
    malloc_block_ints(vals, capacity) &*&
    [_]is_map_keys_equality<t>(keys_eq, kp) &*&
    [_]is_map_key_hash<t>(khash, kp, hsh) &*&
    mapping(?m, ?addrs, kp, recp, hsh, capacity,
            busybits, keyps, khs, chns, vals) &*&
    size == length(m) &*&
    map == mapc(capacity, m, addrs);
  @*/

int map_internal_allocate/*@ <t> @*/(map_keys_equality* keq, map_key_hash* khash,
                            int capacity,
                            struct Map_Internal** map_out)
/*@ requires 0 < capacity &*& capacity < CAPACITY_UPPER_LIMIT &*&
             [_]is_map_keys_equality<t>(keq, ?kp) &*&
             [_]is_map_key_hash<t>(khash, kp, ?hsh) &*&
             *map_out |-> ?old_mo; @*/
/*@ ensures result == 0 ?
              *map_out |-> old_mo :
              (*map_out |-> ?new_mo &*&
               result == 1 &*&
               mapp<t>(new_mo, kp, hsh, nop_true,
                       mapc(capacity, nil, nil))); @*/
{
  struct Map_Internal* old_map_val = *map_out;
  struct Map_Internal* map_alloc = malloc(sizeof(struct Map_Internal));
  if (map_alloc == NULL) return 0;
  *map_out = (struct Map_Internal*) map_alloc;
  int* bbs_alloc = malloc(sizeof(int)*capacity);
  if (bbs_alloc == NULL) {
    free(map_alloc);
    *map_out = old_map_val;
    return 0;
  }
  (*map_out)->busybits = bbs_alloc;
  void** keyps_alloc = malloc(sizeof(void*)*capacity);
  if (keyps_alloc == NULL) {
    free(bbs_alloc);
    free(map_alloc);
    *map_out = old_map_val;
    return 0;
  }
  (*map_out)->keyps = keyps_alloc;
  int* khs_alloc = malloc(sizeof(int)*capacity);
  if (khs_alloc == NULL) {
    free(keyps_alloc);
    free(bbs_alloc);
    free(map_alloc);
    *map_out = old_map_val;
    return 0;
  }
  (*map_out)->khs = khs_alloc;
  int* chns_alloc = malloc(sizeof(int)*capacity);
  if (chns_alloc == NULL) {
    free(khs_alloc);
    free(keyps_alloc);
    free(bbs_alloc);
    free(map_alloc);
    *map_out = old_map_val;
    return 0;
  }
  (*map_out)->chns = chns_alloc;
  int* vals_alloc = malloc(sizeof(int)*capacity);
  if (vals_alloc == NULL) {
    free(chns_alloc);
    free(khs_alloc);
    free(keyps_alloc);
    free(bbs_alloc);
    free(map_alloc);
    *map_out = old_map_val;
    return 0;
  }
  (*map_out)->vals = vals_alloc;
  (*map_out)->capacity = capacity;
  (*map_out)->size = 0;
  (*map_out)->keys_eq = keq;
  (*map_out)->khash = khash;
  //@ close map_key_type<t>();
  //@ close map_key_hash<t>(hsh);
  //@ close map_record_property<t>(nop_true);
  map_impl_init((*map_out)->busybits,
                keq,
                (*map_out)->keyps,
                (*map_out)->khs,
                (*map_out)->chns,
                (*map_out)->vals,
                capacity);
  /*@
    close mapp<t>(*map_out, kp, hsh, nop_true, mapc(capacity, nil, nil));
    @*/
  return 1;
}

/*@
  predicate mapp<t>(struct Map_Internal* ptr,
                    predicate (void*;t) kp,
                    fixpoint (t,int) hsh,
                    fixpoint (t,int,bool) recp,
                    mapi<t> map) =
    malloc_block_Map(ptr) &*&
    ptr->busybits |-> ?busybits &*&
    ptr->keyps |-> ?keyps &*&
    ptr->khs |-> ?khs &*&
    ptr->chns |-> ?chns &*&
    ptr->vals |-> ?vals &*&
    ptr->capacity |-> ?capacity &*&
    ptr->size |-> ?size &*&
    ptr->keys_eq |-> ?keys_eq &*&
    ptr->khash |-> ?khash &*&
    malloc_block_ints(busybits, capacity) &*&
    malloc_block_pointers(keyps, capacity) &*&
    malloc_block_ints(khs, capacity) &*&
    malloc_block_ints(chns, capacity) &*&
    malloc_block_ints(vals, capacity) &*&
    [_]is_map_keys_equality<t>(keys_eq, kp) &*&
    [_]is_map_key_hash<t>(khash, kp, hsh) &*&
    mapping(?m, ?addrs, kp, recp, hsh, capacity,
            busybits, keyps, khs, chns, vals) &*&
    size == length(m) &*&
    map == mapc(capacity, m, addrs);
  @*/

int map_allocate/*@ <t> @*/(map_keys_equality* keq, map_key_hash* khash,
                            int capacity,
                            struct Map** map_out)
/*@ requires 0 < capacity &*& capacity < CAPACITY_UPPER_LIMIT &*&
             [_]is_map_keys_equality<t>(keq, ?kp) &*&
             [_]is_map_key_hash<t>(khash, kp, ?hsh) &*&
             *map_out |-> ?old_mo; @*/
/*@ ensures result == 0 ?
              *map_out |-> old_mo :
              (*map_out |-> ?new_mo &*&
               result == 1 &*&
               mapp<t>(new_mo, kp, hsh, nop_true,
                       mapc(capacity, nil, nil))); @*/
{
  struct Map* old_map_val = *map_out;
  struct Map* map_alloc = malloc(sizeof(struct Map));
  if (map_alloc == NULL) return 0;
  *map_out = (struct Map*) map_alloc;

  if(!map_internal_allocate(keq, khash, capacity, &map_alloc->maps[0])) {
    free(map_alloc);
    return 0;
  }

  if(!map_internal_allocate(keq, khash, capacity, &map_alloc->maps[1])) {
    //map_delete(map_alloc->maps[0]);
    free(map_alloc);
    return 0;
  }

  map_alloc->current_map = 0;
  map_alloc->maps[0]->hash_seed = rand();
  map_alloc->maps[1]->hash_seed = rand();

  return 1;
}

int map_get/*@ <t> @*/(struct Map* map, void* key, int* value_out)
/*@ requires mapp<t>(map, ?kp, ?hsh, ?recp,
                     mapc(?capacity, ?contents, ?addrs)) &*&
             kp(key, ?k) &*&
             *value_out |-> ?old_v; @*/
/*@ ensures mapp<t>(map, kp, hsh, recp,
                    mapc(capacity, contents, addrs)) &*&
            kp(key, k) &*&
            map_has_fp(contents, k) ?
              (result == 1 &*&
               *value_out |-> ?new_v &*&
               new_v == map_get_fp(contents, k)) :
              (result == 0 &*&
               *value_out |-> old_v); @*/
{
#ifdef DUMP_PERF_VARS
 perf_dump_prefix="map_";
 perf_dump_suffix="";
#endif

  //@ open mapp<t>(map, kp, hsh, recp, mapc(capacity, contents, addrs));
  map_key_hash* khash = CURRENT_MAP(map)->khash;
  int hash = khash(key) ^ CURRENT_MAP(map)->hash_seed;
  return map_impl_get(CURRENT_MAP(map)->busybits,
                      CURRENT_MAP(map)->keyps,
                      CURRENT_MAP(map)->khs,
                      CURRENT_MAP(map)->chns,
                      CURRENT_MAP(map)->vals,
                      key,
                      CURRENT_MAP(map)->keys_eq,
                      hash,
                      value_out,
                      CURRENT_MAP(map)->capacity);
  //@ close mapp<t>(map, kp, hsh, recp, mapc(capacity, contents, addrs));
}

void map_put/*@ <t> @*/(struct Map* map, void* key, int value)
/*@ requires mapp<t>(map, ?kp, ?hsh, ?recp,
                     mapc(?capacity, ?contents, ?addrs)) &*&
             [0.5]kp(key, ?k) &*&
             true == recp(k, value) &*&
             length(contents) < capacity &*&
             false == map_has_fp(contents, k); @*/
/*@ ensures mapp<t>(map, kp, hsh, recp,
                    mapc(capacity, map_put_fp(contents, k, value),
                         map_put_fp(addrs, k, key))); @*/
{
#ifdef DUMP_PERF_VARS
 perf_dump_prefix="map_";
 perf_dump_suffix="";
#endif

  int num_traversals = 0;
  //@ open mapp<t>(map, kp, hsh, recp, mapc(capacity, contents, addrs));
  map_key_hash* khash = CURRENT_MAP(map)->khash;
  int hash = khash(key) ^ CURRENT_MAP(map)->hash_seed;
  map_impl_put(CURRENT_MAP(map)->busybits,
               CURRENT_MAP(map)->keyps,
               CURRENT_MAP(map)->khs,
               CURRENT_MAP(map)->chns,
               CURRENT_MAP(map)->vals,
               key, hash, value,
               CURRENT_MAP(map)->capacity,
               &num_traversals);
  ++CURRENT_MAP(map)->size;
  /*@ close mapp<t>(map, kp, hsh, recp, mapc(capacity,
                                             map_put_fp(contents, k, value),
                                             map_put_fp(addrs, k, key)));
  @*/

  if (num_traversals > REHASHING_THRESHOLD) {
    map_impl_init(OTHER_MAP(map)->busybits,
                  OTHER_MAP(map)->keys_eq,
                  OTHER_MAP(map)->keyps,
                  OTHER_MAP(map)->khs,
                  OTHER_MAP(map)->chns,
                  OTHER_MAP(map)->vals,
                  OTHER_MAP(map)->capacity);
    OTHER_MAP(map)->hash_seed = rand();

    for (int i = 0; i < CURRENT_MAP(map)->capacity; i++) {
      if (CURRENT_MAP(map)->busybits[i]) {
        void *key = CURRENT_MAP(map)->keyps[i];
        int value = CURRENT_MAP(map)->vals[i];
        int hash = khash(key) ^ OTHER_MAP(map)->hash_seed;
        map_impl_put(OTHER_MAP(map)->busybits,
                    OTHER_MAP(map)->keyps,
                    OTHER_MAP(map)->khs,
                    OTHER_MAP(map)->chns,
                    OTHER_MAP(map)->vals,
                    key, hash, value,
                    OTHER_MAP(map)->capacity,
                    &num_traversals);
        ++OTHER_MAP(map)->size;
      }
    }

    map->current_map = !(map->current_map);
  }
}

/*@
  lemma void map_erase_decrement_len<kt, vt>(list<pair<kt, vt> > m, kt k)
  requires true == map_has_fp(m, k);
  ensures length(m) == 1 + length(map_erase_fp(m, k));
  {
    switch(m) {
      case nil:
      case cons(h,t):
        switch(h) { case pair(key,val):
          if (key != k) map_erase_decrement_len(t, k);
        }
    }
  }
  @*/

/*@
  lemma void map_get_mem<t>(list<pair<t, int> > m, t k)
  requires true == map_has_fp(m, k);
  ensures true == mem(pair(k, map_get_fp(m, k)), m);
  {
    switch(m) {
      case nil:
      case cons(h,t):
        switch(h) { case pair(key, val):
          if (key != k) map_get_mem(t, k);
        }
    }
  }
  @*/

void map_erase/*@ <t> @*/(struct Map* map, void* key, void** trash)
/*@ requires mapp<t>(map, ?kp, ?hsh, ?recp,
                     mapc(?capacity, ?contents, ?addrs)) &*&
             [0.5]kp(key, ?k) &*&
             *trash |-> _ &*&
             true == map_has_fp(contents, k); @*/
/*@ ensures mapp<t>(map, kp, hsh, recp,
                    mapc(capacity, map_erase_fp(contents, k),
                         map_erase_fp(addrs, k))) &*&
            [0.5]kp(key, k) &*&
            *trash |-> ?k_out &*&
            k_out == map_get_fp(addrs, k) &*&
            [0.5]kp(k_out, k); @*/
{
#ifdef DUMP_PERF_VARS
 perf_dump_prefix="map_";
 perf_dump_suffix="";
#endif

  //@ open mapp<t>(map, kp, hsh, recp, mapc(capacity, contents, addrs));
  map_key_hash* khash = CURRENT_MAP(map)->khash;
  int hash = khash(key) ^ CURRENT_MAP(map)->hash_seed;
  map_impl_erase(CURRENT_MAP(map)->busybits,
                 CURRENT_MAP(map)->keyps,
                 CURRENT_MAP(map)->khs,
                 CURRENT_MAP(map)->chns,
                 key,
                 CURRENT_MAP(map)->keys_eq,
                 hash,
                 CURRENT_MAP(map)->capacity,
                 trash);
  --CURRENT_MAP(map)->size;
  //@ map_erase_decrement_len(contents, k);
  /*@
    close mapp<t>(map, kp, hsh, recp, mapc(capacity,
                                           map_erase_fp(contents, k),
                                           map_erase_fp(addrs, k)));
    @*/
}

int map_size/*@ <t> @*/(struct Map* map)
/*@ requires mapp<t>(map, ?kp, ?hsh, ?recp,
                     mapc(?capacity, ?contents, ?addrs)); @*/
/*@ ensures mapp<t>(map, kp, hsh, recp,
                    mapc(capacity, contents, addrs)) &*&
            result == length(contents); @*/
{
  //@ open mapp<t>(map, kp, hsh, recp, mapc(capacity, contents, addrs));
  return CURRENT_MAP(map)->size;
  //@ close mapp<t>(map, kp, hsh, recp, mapc(capacity, contents, addrs));
}

// bool eq(void *a, void *b) { return *((int *)a) == *((int *)b); }
// 
// int hash(void *k) { return *((int *)k); }
// 
// #include <stdio.h>
// #include <assert.h>
// int main(int argc, char *argv[]) {
//   struct Map *map;
//   int result = map_allocate(eq, hash, 10, &map);
//   assert(result && map);
// 
//   for (int i = 0; i < 10; i++) {
//     int *key = (int *)malloc(sizeof(int));
//     *key = i;
// 
//     map_put(map, key, i);
//     printf("PUT %d - size=%d\n", i, CURRENT_MAP(map)->size);
//   }
// 
//   for (int i = 0; i < 10; i++) {
//     int *key = malloc(sizeof(int));
//     *key = i;
// 
//     int value;
//     result = map_get(map, key, &value);
//     assert(result);
//     printf("GET %d = %d\n", i, value);
//   }
// 
//   for (int i = 0; i < 10; i++) {
//     int *key = malloc(sizeof(int));
//     *key = i;
// 
//     printf("ERASE %d\n", i);
//     void *trash;
//     map_erase(map, key, &trash);
//     free(trash);
//   }
// 
//   for (int i = 0; i < 10; i++) {
//     int *key = malloc(sizeof(int));
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
