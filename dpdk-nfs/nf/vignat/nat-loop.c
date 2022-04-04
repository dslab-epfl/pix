#include "nat-loop.h"
#include "lib/stubs/containers/double-chain-stub-control.h"
#include "lib/stubs/containers/map-stub-control.h"
#include "lib/stubs/containers/vector-stub-control.h"
#include "lib/stubs/time_stub_control.h"
#include <klee/klee.h>

void loop_reset(struct Map **fm, struct Vector **fv, struct DoubleChain **heap,
                int max_flows, int start_port, uint32_t ext_ip,
                uint32_t nat_device, unsigned int lcore_id, time_t *time) {
  map_reset(*fm);
  vector_reset(*fv);
  dchain_reset(*heap, max_flows);
  *time = restart_time();
}
void loop_invariant_consume(struct Map **fm, struct Vector **fv,
                            struct DoubleChain **heap, int max_flows,
                            int start_port, uint32_t ext_ip,
                            uint32_t nat_device, unsigned int lcore_id,
                            time_t time) {}
void loop_invariant_produce(struct Map **fm, struct Vector **fv,
                            struct DoubleChain **heap, int max_flows,
                            int start_port, uint32_t ext_ip,
                            uint32_t nat_device, unsigned int *lcore_id,
                            time_t *time) {}
void loop_iteration_border(struct Map **fm, struct Vector **fv,
                           struct DoubleChain **heap, int max_flows,
                           int start_port, uint32_t ext_ip, uint32_t nat_device,
                           unsigned int lcore_id, time_t time) {
  loop_invariant_consume(fm, fv, heap, max_flows, start_port, ext_ip,
                         nat_device, lcore_id, time);
  loop_reset(fm, fv, heap, max_flows, start_port, ext_ip, nat_device, lcore_id,
             &time);
  loop_invariant_produce(fm, fv, heap, max_flows, start_port, ext_ip,
                         nat_device, &lcore_id, &time);
}
