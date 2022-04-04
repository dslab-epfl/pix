#include "lib/nf_time.h"
#include "time_stub_control.h"
#include <klee/klee.h>

time_t starting_time = 0;
time_t last_time = 0;

time_t start_time(void) {
  klee_make_symbolic(&starting_time, sizeof(time_t), "starting_time");
  klee_possibly_havoc(&starting_time, sizeof(time_t), "starting_time");
  klee_possibly_havoc(&last_time, sizeof(time_t), "last_time");
  klee_assume(starting_time >= 0);
  last_time = starting_time;
  return last_time;
}

time_t restart_time(void) {
  time_t new_time = 0;
  klee_make_symbolic(&new_time, sizeof(time_t), "restarting_time");
  starting_time = new_time;
  klee_assume(starting_time >= 0);
  last_time = starting_time;
  return last_time;
}

time_t current_time(void) {
  time_t prev_time = last_time;
  klee_make_symbolic(&last_time, sizeof(time_t), "next_time");
  klee_assume(prev_time <= last_time);
  return last_time;
}

time_t get_start_time_internal(void) { return starting_time; }
time_t get_start_time(void) { return get_start_time_internal(); }
