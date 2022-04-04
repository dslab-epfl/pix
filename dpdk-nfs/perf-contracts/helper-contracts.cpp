/* Contracts for Helper Functions */

#include "helper-contracts.h"

/*  Perf contracts */

long ether_addr_hash_contract(std::string metric) {
  long constant;
  if (metric == "instruction count")
    constant = 3;
  else if (metric == "memory instructions")
    constant = 3;
  else if (metric == "execution cycles")
    constant = 0 * DRAM_LATENCY + 3 * L1_LATENCY + 0;
  else if (metric == "llvm instruction count")
    constant = 7;
  else if (metric == "llvm memory instructions")
    constant = 2;
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}

long ether_addr_eq_contract(std::string metric, long success) {
  long constant;
  if (metric == "instruction count")
    constant = 31;
  else if (metric == "memory instructions")
    constant = 9;
  else if (metric == "execution cycles")
    constant = 2 * DRAM_LATENCY + 7 * L1_LATENCY + 0;
  else if (metric == "llvm instruction count")
    constant = 2;
  else if (metric == "llvm memory instructions")
    constant = 0;
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}
long flow_id_eq_contract(std::string metric, long success) {
  long constant;
  if (metric == "instruction count")
    constant = 20;
  else if (metric == "memory instructions")
    constant = 12;
  else if (metric == "execution cycles")
    constant = 1 * DRAM_LATENCY + 11 * L1_LATENCY + 17;
  else if (metric == "llvm instruction count")
    constant = 46;
  else if (metric == "llvm memory instructions")
    constant = 12;
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}

long ext_key_eq_contract(std::string metric, long success) {
  long constant;
  if (metric == "instruction count")
    constant = 20;
  else if (metric == "memory instructions")
    constant = 12;
  else if (metric == "execution cycles")
    constant = 1 * DRAM_LATENCY + 11 * L1_LATENCY + 17;
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}

long flow_id_hash_contract(std::string metric) {
  long constant;
  if (metric == "instruction count")
    constant = 45;
  else if (metric == "memory instructions")
    constant = 8;
  else if (metric == "execution cycles")
    constant = 0 * DRAM_LATENCY + 8 * L1_LATENCY + 55;
  else if (metric == "llvm instruction count")
    constant = 35;
  else if (metric == "llvm memory instructions")
    constant = 6;
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}

long ext_key_hash_contract(std::string metric) {
  long constant;
  if (metric == "instruction count")
    constant = 40;
  else if (metric == "memory instructions")
    constant = 8;
  else if (metric == "execution cycles")
    constant = 0 * DRAM_LATENCY + 8 * L1_LATENCY + 55;
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}

long flow_cpy_contract(std::string metric, long recent) {
  long constant;
  if (metric == "instruction count")
    constant = 43;
  else if (metric == "memory instructions")
    constant = 44;
  else if (metric == "execution cycles")
    constant = 42 * L1_LATENCY + 1;
  else {
    assert( 0 && "Contract does not support this metric");
  }
  if (recent)
    constant += 2 * L1_LATENCY;
  else
    constant += 2 * DRAM_LATENCY;
  return constant;
}

long flow_destroy_contract(std::string metric) {
  long constant;
  if (metric == "instruction count")
    constant = 1;
  else if (metric == "memory instructions")
    constant = 2;
  else if (metric == "execution cycles")
    constant = 0 * DRAM_LATENCY + 2 * L1_LATENCY + 1;
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}

long flow_extract_keys_contract(std::string metric) {
  long constant;
  if (metric == "instruction count")
    constant = 4;
  else if (metric == "memory instructions")
    constant = 4;
  else if (metric == "execution cycles")
    constant = 0 * DRAM_LATENCY + 4 * L1_LATENCY + 2;
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}

long flow_pack_keys_contract(std::string metric) {
  long constant;
  if (metric == "instruction count")
    constant = 1;
  else if (metric == "memory instructions")
    constant = 2;
  else if (metric == "execution cycles")
    constant = 0 * DRAM_LATENCY + 2 * L1_LATENCY + 1;
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}

long lb_flow_hash_contract(std::string metric) {
  long constant;
  if (metric == "instruction count")
    constant = 21;
  else if (metric == "memory instructions")
    constant = 5;
  else if (metric == "execution cycles")
    constant = 0 * DRAM_LATENCY + 5 * L1_LATENCY + 16;
  else if (metric == "llvm instruction count")
    constant = 22;
  else if (metric == "llvm memory instructions")
    constant = 4;
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}

long lb_ip_hash_contract(std::string metric) {
  long constant;
  if (metric == "instruction count")
    constant = 2;
  else if (metric == "memory instructions")
    constant = 2;
  else if (metric == "execution cycles")
    constant = 0 * DRAM_LATENCY + 2 * L1_LATENCY;
  else if (metric == "llvm instruction count")
    constant = 3;
  else if (metric == "llvm memory instructions")
    constant = 1;
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}

long lb_flow_equality_contract(std::string metric, long success) {
  long constant;
  if (metric == "instruction count")
    constant = 13;
  else if (metric == "memory instructions")
    constant = 8;
  else if (metric == "execution cycles")
    constant = 0 * DRAM_LATENCY + 8 * L1_LATENCY + 5;
  else if (metric == "llvm instruction count")
    constant = 30;
  else if (metric == "llvm memory instructions")
    constant = 8;
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}

long lb_ip_equality_contract(std::string metric, long success) {
  long constant;
  if (metric == "instruction count")
    constant = 4;
  else if (metric == "memory instructions")
    constant = 3;
  else if (metric == "execution cycles")
    constant = 0 * DRAM_LATENCY + 3 * L1_LATENCY + 1;
  else if (metric == "llvm instruction count")
    constant = 6;
  else if (metric == "llvm memory instructions")
    constant = 2;
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}

long policer_flow_hash_contract(std::string metric) {
  long constant;
  /* Needs to be filled in */
  if (metric == "instruction count")
    constant = 0;
  else if (metric == "memory instructions")
    constant = 0;
  else if (metric == "execution cycles")
    constant = 0 * DRAM_LATENCY + 0 * L1_LATENCY + 0;
  else if (metric == "llvm instruction count")
    constant = 0;
  else if (metric == "llvm memory instructions")
    constant = 0;
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}

long policer_flow_eq_contract(std::string metric, long success) {
  long constant;
  /* Needs to be filled in */
  if (metric == "instruction count")
    constant = 0;
  else if (metric == "memory instructions")
    constant = 0;
  else if (metric == "execution cycles")
    constant = 0 * DRAM_LATENCY + 0 * L1_LATENCY + 0;
  else if (metric == "llvm instruction count")
    constant = 0;
  else if (metric == "llvm memory instructions")
    constant = 0;
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}

/* Concrete state contracts */

std::map<std::string, std::set<int>> ether_addr_hash_cstate_contract() {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>> ether_addr_eq_cstate_contract() {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>> flow_id_hash_cstate_contract() {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>> ext_key_hash_cstate_contract() {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>> flow_id_eq_cstate_contract() {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>> ext_key_eq_cstate_contract() {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>> flow_cpy_cstate_contract() {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>> flow_destroy_cstate_contract() {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>> flow_extract_keys_cstate_contract() {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>> flow_pack_keys_cstate_contract() {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>> lb_flow_hash_cstate_contract() {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>> lb_ip_hash_cstate_contract() {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>> lb_flow_equality_cstate_contract() {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>> lb_ip_equality_cstate_contract() {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>> policer_flow_hash_cstate_contract() {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>> policer_flow_eq_cstate_contract() {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}
/* Perf formula contracts */

perf_formula ether_addr_hash_formula_contract(std::string metric,
                                              PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = ether_addr_hash_contract(metric);
  else if (PCVAbs == FN_CALLS)
    assert(0 && "Internal function should never be called");

  return formula;
}

perf_formula ether_addr_eq_formula_contract(std::string metric, long success,
                                            PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = ether_addr_eq_contract(metric, success);
  else if (PCVAbs == FN_CALLS)
    assert(0 && "Internal function should never be called");
  return formula;
}

perf_formula flow_id_eq_formula_contract(std::string metric, long success,
                                         PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = flow_id_eq_contract(metric, success);
  else if (PCVAbs == FN_CALLS)
    assert(0 && "Internal function should never be called");
  return formula;
}

perf_formula ext_key_eq_formula_contract(std::string metric, long success,
                                         PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = ext_key_eq_contract(metric, success);
  else if (PCVAbs == FN_CALLS)
    assert(0 && "Internal function should never be called");
  return formula;
}

perf_formula flow_id_hash_formula_contract(std::string metric,
                                           PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = flow_id_hash_contract(metric);
  else if (PCVAbs == FN_CALLS)
    assert(0 && "Internal function should never be called");
  return formula;
}

perf_formula ext_key_hash_formula_contract(std::string metric,
                                           PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = ext_key_hash_contract(metric);
  else if (PCVAbs == FN_CALLS)
    assert(0 && "Internal function should never be called");
  return formula;
}

perf_formula flow_cpy_formula_contract(std::string metric, long recent,
                                       PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = flow_cpy_contract(metric, recent);
  else if (PCVAbs == FN_CALLS)
    assert(0 && "Internal function should never be called");
  return formula;
}

perf_formula flow_destroy_formula_contract(std::string metric,
                                           PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = flow_destroy_contract(metric);
  else if (PCVAbs == FN_CALLS)
    assert(0 && "Internal function should never be called");
  return formula;
}

perf_formula flow_extract_keys_formula_contract(std::string metric,
                                                PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = flow_extract_keys_contract(metric);
  else if (PCVAbs == FN_CALLS)
    assert(0 && "Internal function should never be called");
  return formula;
}

perf_formula flow_pack_keys_formula_contract(std::string metric,
                                             PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = flow_pack_keys_contract(metric);
  else if (PCVAbs == FN_CALLS)
    assert(0 && "Internal function should never be called");
  return formula;
}

perf_formula lb_flow_hash_formula_contract(std::string metric,
                                           PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = lb_flow_hash_contract(metric);
  else if (PCVAbs == FN_CALLS)
    assert(0 && "Internal function should never be called");
  return formula;
}

perf_formula lb_ip_hash_formula_contract(std::string metric,
                                         PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = lb_ip_hash_contract(metric);
  else if (PCVAbs == FN_CALLS)
    assert(0 && "Internal function should never be called");
  return formula;
}

perf_formula lb_flow_equality_formula_contract(std::string metric, long success,
                                               PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = lb_flow_equality_contract(metric, success);
  else if (PCVAbs == FN_CALLS)
    assert(0 && "Internal function should never be called");
  return formula;
}

perf_formula lb_ip_equality_formula_contract(std::string metric, long success,
                                             PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = lb_ip_equality_contract(metric, success);
  else if (PCVAbs == FN_CALLS)
    assert(0 && "Internal function should never be called");
  return formula;
}

perf_formula policer_flow_hash_formula_contract(std::string metric,
                                             PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = policer_flow_hash_contract(metric);
  else if (PCVAbs == FN_CALLS)
    assert(0 && "Internal function should never be called");
  return formula;
}

perf_formula policer_flow_eq_formula_contract(std::string metric, long success,
                                             PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = policer_flow_eq_contract(metric, success);
  else if (PCVAbs == FN_CALLS)
    assert(0 && "Internal function should never be called");
  return formula;
}