#include "lpm.h"

#include <rte_ethdev.h>
#include <stdlib.h>

#include <klee/klee.h>

char *prefix; /* For tracing */

void __attribute__((noinline)) lpm_init(const char fname[], void **lpm_out) {
  *lpm_out = malloc(1);
  klee_trace_ret();
}

uint32_t __attribute__((noinline)) lpm_lookup(void *lpm, uint32_t addr) {
  klee_trace_ret();
  int match_len = klee_range(0, 33, "pkt.matched_prefix_len");
  int multi_stage_lookup = 0;
  if (match_len > 24) {
    ds_path_2();
    multi_stage_lookup = 1;
  } else
    ds_path_1();
  TRACE_VAR(multi_stage_lookup, "multi_stage_lookup")
  return klee_range(0, rte_eth_dev_count_avail(), "lpm_next_hop");
}
