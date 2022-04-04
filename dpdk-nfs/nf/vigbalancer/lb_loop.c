#include "lb_loop.h"

#include <klee/klee.h>

#include "lib/stubs/containers/double-chain-stub-control.h"
#include "lib/stubs/containers/map-stub-control.h"
#include "lib/stubs/containers/vector-stub-control.h"
#include "lib/stubs/time_stub_control.h"

void lb_loop_iteration_assumptions(
    struct Map **flow_to_flow_id, struct Vector **flow_heap,
    struct DoubleChain **flow_chain, struct Vector **flow_id_to_backend_id,
    struct Vector **backend_ips, struct Vector **backends,
    struct Map **ip_to_backend_id, struct DoubleChain **active_backends,
    struct Vector **cht, time_t time, uint32_t backend_capacity,
    uint32_t flow_capacity) {
  map_reset(*flow_to_flow_id);
  vector_reset(*flow_heap);
  dchain_reset(*flow_chain, flow_capacity);
  vector_reset(*flow_id_to_backend_id);
  vector_reset(*backend_ips);
  vector_reset(*backends);
  map_reset(*ip_to_backend_id);
  dchain_reset(*active_backends, backend_capacity);
  vector_reset(*cht);
}

void lb_loop_invariant_consume(
    struct Map **flow_to_flow_id, struct Vector **flow_heap,
    struct DoubleChain **flow_chain, struct Vector **flow_id_to_backend_id,
    struct Vector **backend_ips, struct Vector **backends,
    struct Map **ip_to_backend_id, struct DoubleChain **active_backends,
    struct Vector **cht, time_t time, uint32_t backend_capacity,
    uint32_t flow_capacity) {
  klee_trace_ret();
}

void lb_loop_invariant_produce(
    struct Map **flow_to_flow_id, struct Vector **flow_heap,
    struct DoubleChain **flow_chain, struct Vector **flow_id_to_backend_id,
    struct Vector **backend_ips, struct Vector **backends,
    struct Map **ip_to_backend_id, struct DoubleChain **active_backends,
    struct Vector **cht, time_t *time, uint32_t backend_capacity,
    uint32_t flow_capacity) {
  klee_trace_ret();

  lb_loop_iteration_assumptions(flow_to_flow_id, flow_heap, flow_chain,
                                flow_id_to_backend_id, backend_ips, backends,
                                ip_to_backend_id, active_backends, cht, *time,
                                backend_capacity, flow_capacity);
  *time = restart_time();
}

void lb_loop_iteration_border(
    struct Map **flow_to_flow_id, struct Vector **flow_heap,
    struct DoubleChain **flow_chain, struct Vector **flow_id_to_backend_id,
    struct Vector **backend_ips, struct Vector **backends,
    struct Map **ip_to_backend_id, struct DoubleChain **active_backends,
    struct Vector **cht, time_t time, uint32_t backend_capacity,
    uint32_t flow_capacity) {
  lb_loop_invariant_consume(flow_to_flow_id, flow_heap, flow_chain,
                            flow_id_to_backend_id, backend_ips, backends,
                            ip_to_backend_id, active_backends, cht, time,
                            backend_capacity, flow_capacity);
  lb_loop_invariant_produce(flow_to_flow_id, flow_heap, flow_chain,
                            flow_id_to_backend_id, backend_ips, backends,
                            ip_to_backend_id, active_backends, cht, &time,
                            backend_capacity, flow_capacity);
}
