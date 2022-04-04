#include "expirator-contracts.h"

/* Perf contracts */

long expire_items_single_map_contract_0(std::string metric,
                                        std::vector<long> values) {
  long n_expired = values[2];
  long map_hash_id = values[3];
  long map_key_eq = values[4];

  long constant, constant_dependency, dynamic, dynamic_dependency;
  dynamic_dependency = 0;
  if (metric == "instruction count") {
    constant = 38;
    dynamic = 19 * n_expired;
  } else if (metric == "memory instructions") {
    constant = 21;
    dynamic = 4 * n_expired;
  } else if (metric == "execution cycles") {

    constant = 0 * DRAM_LATENCY + 15 * L1_LATENCY +
               22; // Have not gone through patterns here. In progress
    dynamic = (4 * L1_LATENCY + 21) * n_expired;
  } else if (metric == "llvm instruction count") {
    constant = 11; // 4 21
    dynamic = 14 * n_expired + 2; // 12* + 10
  } else if (metric == "llvm memory instructions") {
    constant = 3;
    dynamic = 4 * n_expired;
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  std::vector<long> dchain_vars = {0};
  constant_dependency = dchain_expire_one_index_contract(
      metric, dchain_vars); // Eventually fail to expire
  long num_collisions = values[1];

  if (n_expired > 0) {
    /* Swap before passing to map */
    values[2] = n_expired > 1 ? 1 : 0;
    values[3] = map_hash_id;
    values[4] = map_key_eq;
    // Bounding the unique flows
    values[1] = 0;
    long unique_flow_hashes = (n_expired - num_collisions - 1) > 0
                                  ? (n_expired - num_collisions - 1)
                                  : 0;

    dchain_vars = {1};
    dynamic_dependency += dchain_expire_one_index_contract(metric, dchain_vars);
    dynamic_dependency += map_erase_contract_0(metric, values);
    dynamic_dependency += vector_borrow_contract_0(metric, values);
    dynamic_dependency += vector_return_contract_0(metric, values);

    dynamic_dependency *= unique_flow_hashes;

    values[1] = num_collisions;
    int penalised_flows =
        n_expired >= (num_collisions + 1) ? num_collisions + 1 : n_expired;
    for (int i = 0; i < penalised_flows; i++) {
      dynamic_dependency +=
          dchain_expire_one_index_contract(metric, dchain_vars);
      dynamic_dependency += map_erase_contract_0(metric, values);
      dynamic_dependency += vector_borrow_contract_0(metric, values);
      dynamic_dependency += vector_return_contract_0(metric, values);
      values[0]--;
      values[1]--;

      for (std::vector<long>::iterator iter = values.begin();
           iter != values.end(); ++iter) {
        if (*iter < 0) {
          *iter = 0;
        }
      }
    }
  }
  return (constant + constant_dependency + dynamic + dynamic_dependency);
}

/* Cstate contracts */

std::map<std::string, std::set<int>>
expire_items_single_map_cstate_contract_0(std::vector<long> values) {

  // long n_expired = values[2];

  std::map<std::string, std::set<int>> cstate;
  cstate["rsp"] = {-8, -16, -24, -32, -40, -48, -56, -64};
  std::map<std::string, int> dependency_calls;
  dependency_calls["rsp"] = -64;
  std::vector<long> dchain_vars = {0};
  cstate = add_cstate_dependency(
      cstate, dependency_calls,
      dchain_expire_one_index_cstate_contract(dchain_vars)); /*Final fail*/
  return cstate;
}

/* Perf Formula contracts */

perf_formula expire_items_single_map_formula_contract_0(
    std::string metric, std::vector<long> values, PCVAbstraction PCVAbs) {

  perf_formula formula;
  if (PCVAbs == LOOP_CTRS) {
    long n_expired = values[2];
    (void)n_expired;
    long map_hash_id = values[3];
    long map_key_eq = values[4];
    long constant, nexp_coefficient;
    if (metric == "instruction count") {
      constant = 38;
      nexp_coefficient = 19;
    } else if (metric == "memory instructions") {
      constant = 21;
      nexp_coefficient = 4;
    } else if (metric == "execution cycles") {
      constant = 0 * DRAM_LATENCY + 15 * L1_LATENCY + 22;
      nexp_coefficient = 4 * L1_LATENCY + 21;
    } else if (metric == "llvm instruction count") {
      constant = 11;
      nexp_coefficient = 14; // +2 is missing, but cannot be put here
    } else if (metric == "llvm memory instructions") {
      constant = 3;
      nexp_coefficient = 4;
    }
    else {
    assert( 0 && "Contract does not support this metric");
  }
    perf_formula static_formula, constant_dependency, dynamic_dependency;
    static_formula["constant"] = constant;
    static_formula["e"] = nexp_coefficient;
    std::vector<long> temp_values = {
        0}; /* for when dchain_expire_index fails */
    constant_dependency =
        dchain_expire_one_index_formula_contract(metric, temp_values, PCVAbs);

    temp_values = {1}; /* for when dchain_expire_index is successful */
    dynamic_dependency =
        dchain_expire_one_index_formula_contract(metric, temp_values, PCVAbs);
    /* Switch around values before calling map contracts */
    values[2] = 1; /* Map_key_cached hack */
    values[3] = map_hash_id;
    values[4] = map_key_eq;

    dynamic_dependency = add_perf_formula(
        dynamic_dependency,
        map_erase_formula_contract_0(metric, values, PCVAbs), PCVAbs);
    dynamic_dependency = add_perf_formula(
        dynamic_dependency,
        vector_borrow_formula_contract_0(metric, values, PCVAbs), PCVAbs);
    dynamic_dependency = add_perf_formula(
        dynamic_dependency,
        vector_return_formula_contract_0(metric, values, PCVAbs), PCVAbs);

    formula["e"] = 1;
    formula = multiply_perf_formula(formula, dynamic_dependency, PCVAbs);
    formula = add_perf_formula(formula, static_formula, PCVAbs);
    formula = add_perf_formula(formula, constant_dependency, PCVAbs);
  } else if (PCVAbs == FN_CALLS) {
    formula["expire_items_single_map"] = 1;
  }
  return formula;
}