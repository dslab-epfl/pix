#include "router_options.h"

#include <klee/klee.h>

bool __attribute__((noinline))
handle_packet_timestamp(struct ipv4_hdr *header, uint32_t router_ip,
                        uint32_t current_milliseconds_utc) {
  klee_trace_ret();
  return klee_int("timestamp_not_full") != 0;
}
