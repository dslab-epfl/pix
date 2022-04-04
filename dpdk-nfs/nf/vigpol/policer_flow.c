#include "policer_flow.h"
#include "limits.h"
#include "lib/ignore.h"


bool policer_flow_eq(void* a, void* b)
{
  uint32_t* ip1 = (uint32_t*) a;
  uint32_t* ip2 = (uint32_t*) b;
  return *ip1 == *ip2;
}

int policer_flow_hash(void* obj)
{
  uint32_t* ip = (uint32_t*) obj;

  long long hash = 0;
  hash += *ip;
  hash *= 31;
  hash = hash % INT_MAX;

  return (int)hash;
}

void policer_flow_allocate(void* obj)
{
  IGNORE(obj);
}

bool policer_bucket_sanity_check(void* bucket, void* unused){
  struct Bucket* b = (struct Bucket*) bucket;
  return b->bucket_time >=0;
}
