#include "dchain-contracts.h"

/* Perf contracts */
long dchain_is_index_allocated_contract_0(std::string metric,
                                          std::vector<long> values) {
  long constant;
  if (metric == "instruction count") {
    constant = 13; // 2 //2
  } else if (metric == "memory instructions") {
    constant = 4;
  } else if (metric == "execution cycles") {
    constant = (1) * DRAM_LATENCY + 3 * L1_LATENCY +
               9; // Have not gone through patterns here. In progress
  } else if (metric == "llvm instruction count") {
    constant = 15; 
  } else if (metric == "llvm memory instructions") {
    constant = 4;
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}

long dchain_expire_one_index_contract(std::string metric,
                                      std::vector<long> values) {
  long success = values[0];
  long constant;
  if (metric == "instruction count") {
    if (success) {
      constant = 50; 
    } else {
      constant = 32;
    }
  } else if (metric == "memory instructions") {
    if (success) {
      constant = 25;
    } else {
      constant = 18;
    }
  } else if (metric == "execution cycles") {
    if (success) {
      constant = (2) * DRAM_LATENCY + 23 * L1_LATENCY + 28; // Have not gone
                                                            // through patterns
                                                            // here. In progress
                                                            // - Not of much use
    } else // We can reduce this to 9 if we condition on empty list
    {
      constant = (2) * DRAM_LATENCY + 13 * L1_LATENCY +
                 16; // Have not gone through patterns here. In progress
    }
  } else if( metric == "llvm instruction count") {
    if(success) {
      constant = 47 ; // 3 8 16 19 + //impl_get_old 2 6 8 + //impl_free
    } else {
      constant = 24; // 3 8 19 + //impl_get_old 2 6 8
    }
  } else if( metric == "llvm memory instructions") {
    if(success) {
      constant = 16;
    } else {
      constant = 6;
    }
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}

long dchain_allocate_contract_0(std::string metric, std::vector<long> values) {
  return 0;
}
long dchain_allocate_contract_1(std::string metric, std::vector<long> values) {
  return 0;
}
long dchain_allocate_new_index_contract_0(std::string metric,
                                          std::vector<long> values) {
  long success = values[0];
  assert(success == 0); /* success is the inverse here*/
  long constant;
  if (metric == "instruction count") {
    constant = 44; // 3 8 14 // 2 6 18
  } else if (metric == "memory instructions") {
    constant = 27;
  } else if (metric == "execution cycles") {
    constant = (3) * DRAM_LATENCY + 20 * L1_LATENCY + 22;
  } else if (metric == "llvm instruction count") {
    constant = 38; 
  } else if (metric == "llvm memory instructions") {
    constant = 16;
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}
long dchain_allocate_new_index_contract_1(std::string metric,
                                          std::vector<long> values) {
  long success = values[0];
  assert(success); /* success is the inverse here*/
  long constant;
  if (metric == "instruction count") {
    constant = 19; 
  } else if (metric == "memory instructions") {
    constant = 10;
  } else if (metric == "execution cycles") {
    constant = (3) * DRAM_LATENCY + 7 * L1_LATENCY +
               11; // Have not gone through patterns here. In progress
  } else if (metric == "llvm instruction count") {
    constant = 38; 
  } else if (metric == "llvm memory instructions") {
    constant = 16;
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}
long dchain_rejuvenate_index_contract_0(std::string metric,
                                        std::vector<long> values) {
  long constant;
  if (metric == "instruction count") {
    constant = 34; // 3 8 13 // 2 13 22
  } else if (metric == "memory instructions") {
    constant = 20;
  } else if (metric == "execution cycles") {
    constant = (3) * DRAM_LATENCY + 17 * L1_LATENCY + 18;
  } else if (metric == "llvm instruction count") {
    constant = 37; 
  } else if (metric == "llvm memory instructions") {
    constant = 12;
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}

/* Cstate contracts */

std::map<std::string, std::set<int>>
dchain_is_index_allocated_cstate_contract_0(std::vector<long> values) {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}
std::map<std::string, std::set<int>>
dchain_expire_one_index_cstate_contract(std::vector<long> values) {

  long success = values[0];
  std::map<std::string, std::set<int>> cstate;
  if (!success) {
    cstate["rsp"] = {-8, -16, -24, -32, -32};
  }
  return cstate;
}
std::map<std::string, std::set<int>>
dchain_allocate_cstate_contract_0(std::vector<long> values) {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}
std::map<std::string, std::set<int>>
dchain_allocate_new_index_cstate_contract_0(std::vector<long> values) {

  long success = values[0];
  assert(success == 0); /* success is the inverse here*/

  std::map<std::string, std::set<int>> cstate;
  cstate["rsp"] = {-8, -16, -24, -32, -40};
  return cstate;
}
std::map<std::string, std::set<int>>
dchain_allocate_new_index_cstate_contract_1(std::vector<long> values) {

  long success = values[0];
  assert(success); /* success is the inverse here*/

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}
std::map<std::string, std::set<int>>
dchain_rejuvenate_index_cstate_contract_0(std::vector<long> values) {

  std::map<std::string, std::set<int>> cstate;
  cstate["rsp"] = {-8, -16, -24, -32};
  return cstate;
}

/* Perf formula contracts */

perf_formula dchain_expire_one_index_formula_contract(std::string metric,
                                                      std::vector<long> values,
                                                      PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = dchain_expire_one_index_contract(metric, values);
  else if (PCVAbs == FN_CALLS)
    assert(0 && "Internal function should never be called");

  return formula;
}

perf_formula dchain_is_index_allocated_formula_contract_0(
    std::string metric, std::vector<long> values, PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = dchain_is_index_allocated_contract_0(metric, values);
  else if (PCVAbs == FN_CALLS)
    formula["dchain_is_index_allocated"] = 1;

  return formula;
}

perf_formula dchain_allocate_formula_contract_0(std::string metric,
                                                std::vector<long> values,
                                                PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = 0;
  return formula;
}

perf_formula dchain_allocate_new_index_formula_contract_0(
    std::string metric, std::vector<long> values, PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = dchain_allocate_new_index_contract_0(metric, values);
  else if (PCVAbs == FN_CALLS)
    formula["dchain_allocate_new_index_success"] = 1;
  return formula;
}

perf_formula dchain_allocate_new_index_formula_contract_1(
    std::string metric, std::vector<long> values, PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = dchain_allocate_new_index_contract_1(metric, values);
  else if (PCVAbs == FN_CALLS)
    formula["dchain_allocate_new_index_failure"] = 1;
  return formula;
}

perf_formula dchain_rejuvenate_index_formula_contract_0(
    std::string metric, std::vector<long> values, PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = dchain_rejuvenate_index_contract_0(metric, values);
  else if (PCVAbs == FN_CALLS)
    formula["dchain_rejuvenate_index"] = 1;
  return formula;
}
