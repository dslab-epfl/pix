#include "dchain-contracts.h"

/* Perf contracts */

long dchain_is_index_allocated_contract_0(std::string metric,
                                          std::vector<long> values) {
  assert(0 && "dchain_is_index_allocated is not supported by alt-chain");
}
long dchain_expire_one_index_contract(std::string metric,
                                      std::vector<long> values) {
  long success = values[0];
  long constant;
  if (metric == "instruction count") {
    if (success) {
      constant = 25;
    } else {
      constant = 15;
    }
  } else if (metric == "memory instructions") {
    if (success) {
      constant = 10;
    } else {
      constant = 5;
    }
  } else if (metric == "execution cycles") {
    if (success) {
      constant = (9) * DRAM_LATENCY + 16 * L1_LATENCY + 28; // Have not gone
                                                            // through patterns
                                                            // here. In progress
                                                            // - Not of much use
    } else // We can reduce this to 9 if we condition on empty list
    {
      constant = (3) * DRAM_LATENCY + 2 * L1_LATENCY +
                 18; // Have not gone through patterns here. In progress
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
  assert(success == 0);
  long constant;
  if (metric == "instruction count") {
    constant = 36;
  } else if (metric == "memory instructions") {
    constant = 14;
  } else if (metric == "execution cycles") {
    constant = (5) * DRAM_LATENCY + 9 * L1_LATENCY + 38;
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}
long dchain_allocate_new_index_contract_1(std::string metric,
                                          std::vector<long> values) {
  long success = values[0];
  assert(success);
  long constant;
  if (metric == "instruction count") {
    constant = 393224;
  } else if (metric == "memory instructions") {
    constant = 65540;
  } else if (metric == "execution cycles") {
    constant = (3) * DRAM_LATENCY + 7 * L1_LATENCY +
               11; // Have not gone through patterns here. In progress
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
    constant = 29;
  } else if (metric == "memory instructions") {
    constant = 14;
  } else if (metric == "execution cycles") {
    constant = (9) * DRAM_LATENCY + 11 * L1_LATENCY +
               18; // Have not gone through patterns here. In progress
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}

/* Cstate contracts */

std::map<std::string, std::set<int>>
dchain_is_index_allocated_cstate_contract_0(std::vector<long> values) {
  assert(0 && "dchain_is_index_allocated is not supported by alt-chain");
}

std::map<std::string, std::set<int>>
dchain_expire_one_index_cstate_contract(std::vector<long> values) {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>>
dchain_allocate_cstate_contract_0(std::vector<long> values) {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}
std::map<std::string, std::set<int>>
dchain_allocate_cstate_contract_1(std::vector<long> values) {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>>
dchain_allocate_new_index_cstate_contract_0(std::vector<long> values) {

  long success = values[0];
  assert(success == 0);

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}
std::map<std::string, std::set<int>>
dchain_allocate_new_index_cstate_contract_1(std::vector<long> values) {

  long success = values[0];
  assert(success);

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}
std::map<std::string, std::set<int>>
dchain_rejuvenate_index_cstate_contract_0(std::vector<long> values) {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}