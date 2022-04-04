#include <stdint.h>
#include <string.h>
#include <math.h>
#include "pred-map-impl.h"
#include "lib/ignore.h"

#ifdef DUMP_PERF_VARS
#include "lib/nf_log.h"
char *perf_dump_suffix = "";
char *perf_dump_prefix = "";
#endif

#ifdef DUMP_PERF_VARS

int first_empty(int *chns, int capacity)
{
  int i;
  for (i = 0; i < capacity; i++)
  {
    if (chns[i] == 0)
    {
      return i;
    }
  }
  return i;
}

int longest_subchain(int *busybits, int capacity)
{
  int i, start, index;
  int curr = 0;
  int longest = 0;
  start = first_empty(busybits, capacity);
  for (i = 0; i < capacity; i++)
  {
    index = (start + i) % capacity;
    if (buckets[index].busybit != 0)
    {
      curr = curr + 1;
    }
    else
    {
      if (curr > longest)
      {
        longest = curr;
        curr = 0;
      }
    }
  }
  return longest;
}

int longest_chain(int *chns, int capacity)
{
  int i, start, index;
  int curr = 0;
  int longest = 0;
  start = first_empty(chns, capacity);
  for (i = 0; i < capacity; i++)
  {
    index = (start + i) % capacity;
    if (buckets[index].chain != 0)
    {
      curr = curr + 1;
    }
    else
    {
      if (curr > longest)
      {
        longest = curr;
        curr = 0;
      }
    }
  }
  return longest;
}

int longest_collision_chain(int *chns, int *key_hashes, int *bbts, int capacity)
{
  int i, start, index;
  int curr[capacity]; //Hash is always modulo capacity
  for (i = 0; i < capacity; i++)
  {
    curr[i] = 0;
  }
  int max = 0;
  int longest = 0;
  start = first_empty(chns, capacity);
  for (i = 0; i < capacity; i++)
  {
    index = (start + i) % capacity;
    int key_hash = (key_hashes[index] % capacity + capacity) % capacity;
    if (buckets[index].chain != 0 && bbts[index])
    {
      ++curr[key_hash];
      if (curr[key_hash] > max)
      {
        max = curr[key_hash];
      }
      else
      {
        if (max > longest)
        {
          longest = max;
          max = 0;
          for (i = 0; i < capacity; i++)
          {
            curr[i] = 0;
          }
        }
      }
    }
  }
  return longest;
}
#endif //DUMP_PERF_VARS

static int loop(int k, int capacity)
{

  int g = k & (capacity - 1);
  int res = (g + capacity) & (capacity - 1);
  return res;
}

static int find_key(struct Bucket *buckets,
                    void *keyp, map_keys_equality *eq, int key_hash,
                    int capacity)
{
#ifdef DUMP_PERF_VARS
  int buckets_traversed = 0;
  int hash_collisions = 0;
#endif
  int start = loop(key_hash, capacity);
  int i = 0;
  for (; i < capacity; ++i)
  {
#ifdef DUMP_PERF_VARS
    buckets_traversed = buckets_traversed + 1;
#endif
    int index = loop(start + i, capacity);
    int bb = buckets[index].busybit;
    int kh = buckets[index].hash;
    int chn = buckets[index].chain;
    void *kp = buckets[index].key;
    if (bb != 0 && kh == key_hash)
    {
      if (eq(kp, keyp))
      {
#ifdef DUMP_PERF_VARS
        NF_PERF_DEBUG("Map_impl_get:Success:Num_bucket_traversals%s:%d", perf_dump_suffix, buckets_traversed);
        NF_PERF_DEBUG("Map_impl_get:Success:Num_hash_collisions%s:%d", perf_dump_suffix, hash_collisions);
#endif
        return index;
      }
#ifdef DUMP_PERF_VARS
      hash_collisions = hash_collisions + 1;
#endif
    }
    else
    {
      if (chn == 0)
      {
#ifdef DUMP_PERF_VARS
        NF_PERF_DEBUG("Map_impl_get:Truncated fail:Num_bucket_traversals%s:%d", perf_dump_suffix, buckets_traversed);
        NF_PERF_DEBUG("Map_impl_get:Truncated fail:Num_hash_collisions%s:%d", perf_dump_suffix, hash_collisions);
#endif
        return -1;
      }
    }
  }
#ifdef DUMP_PERF_VARS
  NF_PERF_DEBUG("Map_impl_get:Fail:Num_bucket_traversals%s:%d", perf_dump_suffix, buckets_traversed);
  NF_PERF_DEBUG("Map_impl_get:Fail:Num_hash_collisions%s:%d", perf_dump_suffix, hash_collisions);
#endif
  return -1;
}

static int find_key_remove_chain(struct Bucket *buckets,
                                 void *keyp, map_keys_equality *eq,
                                 int key_hash,
                                 int capacity,
                                 void **keyp_out)
{
#ifdef DUMP_PERF_VARS
  int buckets_traversed = 0;
  int hash_collisions = 0;
#endif
  int i = 0;
  int start = loop(key_hash, capacity);
  for (; i < capacity; ++i)
  {
#ifdef DUMP_PERF_VARS
    buckets_traversed = buckets_traversed + 1;
#endif
    int index = loop(start + i, capacity);
    int bb = buckets[index].busybit;
    int kh = buckets[index].hash;
    int chn = buckets[index].chain;
    void *kp = buckets[index].key;
    if (bb != 0 && kh == key_hash)
    {
      if (eq(kp, keyp))
      {
        buckets[index].busybit = 0;
        *keyp_out = buckets[index].key;
#ifdef DUMP_PERF_VARS
        NF_PERF_DEBUG("Map_impl_erase:Success:Num_bucket_traversals%s:%d", perf_dump_suffix, buckets_traversed);
        NF_PERF_DEBUG("Map_impl_erase:Success:Num_hash_collisions%s:%d", perf_dump_suffix, hash_collisions);
#endif
        return index;
      }
#ifdef DUMP_PERF_VARS
      hash_collisions = hash_collisions + 1;
#endif
    }
    buckets[index].chain = chn - 1;
  }

#ifdef DUMP_PERF_VARS
  NF_PERF_DEBUG("Map_impl_erase:Fail:Num_bucket_traversals%s:%d", perf_dump_suffix, buckets_traversed);
  NF_PERF_DEBUG("Map_impl_erase:Fail:Num_hash_collisions%s:%d", perf_dump_suffix, hash_collisions);
#endif
  return -1;
}

static int find_empty(struct Bucket *buckets, int start, int capacity)
{
#ifdef DUMP_PERF_VARS
  int buckets_traversed = 0;
#endif
  int i = 0;
  for (; i < capacity; ++i)
  {
    int index = loop(start + i, capacity);
#ifdef DUMP_PERF_VARS
    buckets_traversed = buckets_traversed + 1;
#endif
    int bb = buckets[index].busybit;
    if (0 == bb)
    {
#ifdef DUMP_PERF_VARS
      NF_PERF_DEBUG("Map_impl_put:Success:Num_bucket_traversals%s:%d", perf_dump_suffix, buckets_traversed);
#endif
      return index;
    }
    int chn = buckets[index].chain;
    buckets[index].chain = chn + 1;
  }
#ifdef DUMP_PERF_VARS
  NF_PERF_DEBUG("Map_impl_put:Fail:Num_bucket_traversals%s:%d", perf_dump_suffix, buckets_traversed);
#endif
  return -1;
}

void map_impl_init(struct Bucket *buckets, map_keys_equality *eq,
                   int capacity)
{
  IGNORE(eq);
  IGNORE(buckets);

  int i = 0;
  for (; i < capacity; ++i)
  {
    buckets[i].busybit = 0;
    buckets[i].chain = 0;
  }
}

int map_impl_get(struct Bucket *buckets,
                 void *keyp, map_keys_equality *eq,
                 int hash, int *value,
                 int capacity)
{

#ifdef DUMP_PERF_VARS
  int occupancy = map_impl_size(busybits, capacity);
  NF_PERF_DEBUG("Map_impl_get:%soccupancy:%d", perf_dump_prefix, occupancy);
  int long_chain = longest_chain(chns, capacity);
  NF_PERF_DEBUG("Map_impl_get:Longest_chain%s:%d", perf_dump_suffix, long_chain);
  int longest_cchain = longest_collision_chain(chns, k_hashes, busybits, capacity);
  NF_PERF_DEBUG("Map_impl_get:Longest_collison_chain%s:%d", perf_dump_suffix, longest_cchain);
#endif

  int index = find_key(buckets, keyp, eq, hash, capacity);
  if (-1 == index)
  {
    return 0;
  }
  *value = buckets[index].value;
  return 1;
}

void map_impl_put(struct Bucket *buckets,
                  void *keyp, int hash, int value,
                  int capacity)
{

#ifdef DUMP_PERF_VARS
  int occupancy = map_impl_size(busybits, capacity);
  NF_PERF_DEBUG("Map_impl_put:%soccupancy:%d", perf_dump_prefix, occupancy);
  int longest_schain = longest_subchain(busybits, capacity);
  NF_PERF_DEBUG("Map_impl_put:Longest_subchain%s:%d", perf_dump_suffix, longest_schain);
#endif

  int start = loop(hash, capacity);
  int index = find_empty(buckets, start, capacity);

  buckets[index].busybit = 1;
  buckets[index].key = keyp;
  buckets[index].hash = hash;
  buckets[index].value = value;
#ifdef FREUD
  buckets[index].traversals = index = start;
#endif //FREUD
}

void map_impl_erase(struct Bucket *buckets, void *keyp,
                    map_keys_equality *eq, int hash, int capacity,
                    void **keyp_out)
{

#ifdef DUMP_PERF_VARS
  int occupancy = map_impl_size(busybits, capacity);
  NF_PERF_DEBUG("Map_impl_erase:%soccupancy:%d", perf_dump_prefix, occupancy);
  int long_chain = longest_chain(chns, capacity);
  NF_PERF_DEBUG("Map_impl_erase:Longest_chain%s:%d", perf_dump_suffix, long_chain);
  int longest_cchain = longest_collision_chain(chns, k_hashes, busybits, capacity);
  NF_PERF_DEBUG("Map_impl_erase:Longest_collison_chain%s:%d", perf_dump_suffix, longest_cchain);
#endif

  find_key_remove_chain(buckets, keyp, eq, hash, capacity, keyp_out);
}

int map_impl_size(struct Bucket *buckets, int capacity)
{

  int s = 0;
  int i = 0;
  for (; i < capacity; ++i)
  {
    if (buckets[i].busybit != 0)
    {
      ++s;
    }
  }

  return s;
}