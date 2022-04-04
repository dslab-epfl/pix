#include "vector-contracts.h"

/* Perf contracts*/

long vector_allocate_contract_0(std::string metric, std::vector<long> values) {
  return 0;
}

long vector_borrow_contract_0(std::string metric, std::vector<long> values) {
  long constant;
  if (metric == "instruction count") {
    constant = 16;
  } else if (metric == "memory instructions") {
    constant = 11;
  } else if (metric == "execution cycles") {
    constant = 1 * DRAM_LATENCY + 4 * L1_LATENCY + 2;
  } else if (metric == "llvm instruction count") {
    constant = 9;
  } else if (metric == "llvm memory instructions") {
    constant = 3;
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}

long vector_return_contract_0(std::string metric, std::vector<long> values) {
  long constant;
  if (metric == "instruction count") {
    constant = 1;
  } else if (metric == "memory instructions") {
    constant = 2;
  } else if (metric == "execution cycles") {
    constant = 1 + 2 * L1_LATENCY;
  } else if (metric == "llvm instruction count") {
    constant = 1;
  } else if (metric == "llvm memory instructions") {
    constant = 0;
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}

/* Cstate contracts */

std::map<std::string, std::set<int>>
vector_allocate_cstate_contract_0(std::vector<long> values) {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>>
vector_borrow_cstate_contract_0(std::vector<long> values) {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>>
vector_return_cstate_contract_0(std::vector<long> values) {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

/* Perf Formula contracts */

perf_formula vector_allocate_formula_contract_0(std::string metric,
                                                std::vector<long> values,
                                                PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = 0;
  return formula;
}

perf_formula vector_borrow_formula_contract_0(std::string metric,
                                              std::vector<long> values,
                                              PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = vector_borrow_contract_0(metric, values);
  else if (PCVAbs == FN_CALLS)
    formula["vector_borrow"] = 1;
  return formula;
}

perf_formula vector_return_formula_contract_0(std::string metric,
                                              std::vector<long> values,
                                              PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = vector_return_contract_0(metric, values);
  else if (PCVAbs == FN_CALLS)
    formula["vector_return"] = 1;
  return formula;
}