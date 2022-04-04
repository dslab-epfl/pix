#ifndef _LOOP_H_INCLUDED_
#define _LOOP_H_INCLUDED_
#include "lib/containers/double-chain.h"
#include "lib/containers/map.h"
#include "lib/containers/vector.h"
#include "lib/coherence.h"
#include "lib/nf_time.h"
#include "nat-flow.h"
/*@ predicate evproc_loop_invariant(struct Map* fm,
                                    struct Vector* fv,
                                    struct DoubleChain* heap,
                                    int max_flows,
                                    int start_port,
                                    int ext_ip,
                                    int nat_device,
                                    unsigned int lcore_id,
                                    time_t time) = 
              mapp<FlowIdi>(fm, FlowIdp, _FlowId_hash, nop_true, mapc(max_flows, ?_fm, ?_fm_addrs)) &*&
              vectorp<FlowIdi>(fv, FlowIdp, ?_fv, ?_fv_addrs) &*&
              length(_fv) == max_flows &*&
              double_chainp(?_heap, heap) &*&
              dchain_high_fp(_heap) <= time &*&
              dchain_index_range_fp(_heap) == max_flows &*&
              max_flows < INT_MAX &*&
              start_port < INT_MAX &*&
              ext_ip < INT_MAX &*&
              nat_device < INT_MAX &*&
              map_vec_chain_coherent<FlowIdi>(_fm, _fv, _heap) &*&
              true == forall2(_fv, _fv_addrs, (kkeeper)(_fm_addrs)) &*&
              lcore_id == 0 &*&
              last_time(time);
@*/
void loop_invariant_consume(struct Map** fm,
                            struct Vector** fv,
                            struct DoubleChain** heap,
                            int max_flows,
                            int start_port,
                            uint32_t ext_ip,
                            uint32_t nat_device,
                            unsigned int lcore_id,
                            time_t time);
/*@ requires *fm |-> ?_fm &*& *fv |-> ?_fv &*& *heap |-> ?_heap &*& 
             evproc_loop_invariant(_fm, _fv, _heap, max_flows, start_port, ext_ip, nat_device, lcore_id, time); @*/
/*@ ensures *fm |-> _fm &*& *fv |-> _fv &*& *heap |-> _heap &*& true; @*/
void loop_invariant_produce(struct Map** fm,
                            struct Vector** fv,
                            struct DoubleChain** heap,
                            int max_flows,
                            int start_port,
                            uint32_t ext_ip,
                            uint32_t nat_device,
                            unsigned int* lcore_id,
                            time_t* time);
/*@ requires *fm |-> ?_fm &*& *fv |-> ?_fv &*& *heap |-> ?_heap &*& *lcore_id |-> _ &*& *time |-> _;@*/
/*@ ensures *fm |-> _fm &*& *fv |-> _fv &*& *heap |-> _heap &*& *lcore_id |-> ?lcid &*& *time |-> ?t &*&
             evproc_loop_invariant(_fm, _fv, _heap, max_flows, start_port, ext_ip, nat_device, lcid, t); @*/

void loop_iteration_border(struct Map** fm,
                           struct Vector** fv,
                           struct DoubleChain** heap,
                           int max_flows,
                           int start_port,
                           uint32_t ext_ip,
                           uint32_t nat_device,
                           unsigned int lcore_id,
                           time_t time);

#endif//_LOOP_H_INCLUDED_
