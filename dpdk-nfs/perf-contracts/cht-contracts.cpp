#include "cht-contracts.h"

/* Perf contracts */

long lb_find_preferred_available_backend_contract_0(std::string metric,
                                                    std::vector<long> values) {
  // long available_backends = values[0];
  long constant = 0;
  if (metric == "instruction count") {
    constant = 55;
  } else if (metric == "memory instructions") {
    constant = 25;
  } else if (metric == "execution cycles") {
    constant = 0 * DRAM_LATENCY + 25 * L1_LATENCY + 31;
  }
  else if (metric == "llvm instruction count")
    constant = 32;
  else if (metric == "llvm memory instructions")
    constant = 5;
  else {
    assert( 0 && "Contract does not support this metric");
  }
  long constant_dependency = vector_borrow_contract_0(metric, values);
  constant_dependency += lb_flow_hash_contract(metric);
  constant_dependency += vector_return_contract_0(metric, values);
  constant_dependency += dchain_is_index_allocated_contract_0(metric, values);
  return constant +
         constant_dependency; /*Hack, assuming backends table is full*/
}

/* Cstate contracts */
std::map<std::string, std::set<int>>
lb_find_preferred_available_backend_cstate_contract_0(
    std::vector<long> values) {

  // long available_backends = values[0];
  std::map<std::string, std::set<int>> cstate;

  cstate["rsp"] = {-8, -16, -24, -32, -40, -48, -64, -72, -80, -96};
  std::map<std::string, int> dependency_calls;
  dependency_calls["rsp"] = -96;
  cstate = add_cstate_dependency(
      cstate, dependency_calls,
      dchain_is_index_allocated_cstate_contract_0(values));
  cstate = add_cstate_dependency(cstate, dependency_calls,
                                 vector_borrow_cstate_contract_0(values));
  cstate = add_cstate_dependency(cstate, dependency_calls,
                                 vector_return_cstate_contract_0(values));
  return cstate;
}

/* Perf Formula contracts */
perf_formula lb_find_preferred_available_backend_formula_contract_0(
    std::string metric, std::vector<long> values, PCVAbstraction PCVAbs) {

  perf_formula formula;
  if (PCVAbs == LOOP_CTRS) {

    long constant = 0;
    if (metric == "instruction count") {
      constant = 55;
    } else if (metric == "memory instructions") {
      constant = 25;
    } else if (metric == "execution cycles") {
      constant = 0 * DRAM_LATENCY + 25 * L1_LATENCY + 31;
    }
    else if (metric == "llvm instruction count")
      constant = 32;
    else if (metric == "llvm memory instructions")
      constant = 5;
    else {
    assert( 0 && "Contract does not support this metric");
  }
    formula["constant"] = constant;

    /* Add dependencies */
    formula = add_perf_formula(
        formula, vector_borrow_formula_contract_0(metric, values, PCVAbs),
        PCVAbs);
    formula = add_perf_formula(
        formula, vector_return_formula_contract_0(metric, values, PCVAbs),
        PCVAbs);
    formula = add_perf_formula(
        formula,
        lb_flow_hash_formula_contract(metric, PCVAbs),
        PCVAbs);
    formula = add_perf_formula(
        formula,
        dchain_is_index_allocated_formula_contract_0(metric, values, PCVAbs),
        PCVAbs);
  } else if (PCVAbs == FN_CALLS) {
    formula["map_put"] = 1;
  }

  return formula;
}