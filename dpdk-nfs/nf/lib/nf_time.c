#include "nf_time.h"

#undef time_t // catch potential mismatch
#include <time.h>

time_t current_time(void)
//@ requires last_time(?x);
//@ ensures result >= 0 &*& x <= result &*& last_time(result);
{
  struct timespec tp;
  clock_gettime(CLOCK_MONOTONIC, &tp);
  /* ns granularity */
  return (tp.tv_sec*1e9 + tp.tv_nsec);
  /* us granularity */
  // return tp.tv_sec*1e6+ tp.tv_nsec/1e3;
}
