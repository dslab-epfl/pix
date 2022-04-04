#ifndef _FLOW_H_INCLUDED_
#define _FLOW_H_INCLUDED_
#include "lib/ignore.h"
#include <stdbool.h>
#include <stdint.h>
#include "lib/nf_time.h"

#ifdef KLEE_VERIFICATION
#define AND &
#else // KLEE_VERIFICATION
#define AND &&
#endif // KLEE_VERIFICATION

struct Bucket {
  uint64_t bucket_size;
  time_t bucket_time;
};

int policer_flow_hash(void *obj);

bool policer_flow_eq(void *a, void *b);

void policer_flow_allocate(void *obj);

bool policer_bucket_sanity_check(void* bucket, void* unused);

#ifdef KLEE_VERIFICATION
#include "lib/stubs/containers/str-descr.h"
#include <klee/klee.h>

#endif // KLEE_VERIFICATION

#endif //_FLOW_H_INCLUDED_