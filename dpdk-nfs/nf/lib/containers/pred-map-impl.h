#pragma once 

#include <stdbool.h>

struct Bucket //This is exactly 32Bytes, a submultiple of the cache line size
{
  int busybit;
  void *key;
  int hash;
  int chain;
  int value;
#ifdef FREUD
  int traversals;
#endif
};

typedef bool map_keys_equality(void *k1, void *k2);

void map_impl_init(struct Bucket *buckets, map_keys_equality *cmp,
                   int capacity);

int map_impl_get(struct Bucket *buckets,
                 void *keyp, map_keys_equality *eq,
                 int hash, int *value,
                 int capacity);

void map_impl_put(struct Bucket *buckets,
                  void *keyp, int hash, int value,
                  int capacity);

void map_impl_erase(struct Bucket *buckets, void *keyp,
                    map_keys_equality *eq, int hash,
                    int capacity,
                    void **keyp_out);

int map_impl_size(struct Bucket *buckets, int capacity);