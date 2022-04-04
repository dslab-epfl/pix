#include "map-contracts.h"

std::map<long, map_hash_ptr> hash_ptr_map = {
    {1, &flow_id_hash_contract},
    {2, &ether_addr_hash_contract},
    {3, &lb_flow_hash_contract},
    {4, &lb_ip_hash_contract},
    {5, &policer_flow_hash_contract},
};

std::map<long, helper_cstate_fn_ptr> hash_cstate_ptr_map = {
    {1, &flow_id_hash_cstate_contract},
    {2, &ether_addr_hash_cstate_contract},
    {3, &lb_flow_hash_cstate_contract},
    {4, &lb_ip_hash_cstate_contract},
    {5, &policer_flow_hash_cstate_contract},
};

std::map<long, map_hash_formula_ptr> hash_formula_ptr_map = {
    {1, &flow_id_hash_formula_contract},
    {2, &ether_addr_hash_formula_contract},
    {3, &lb_flow_hash_formula_contract},
    {4, &lb_ip_hash_formula_contract},
    {5, &policer_flow_hash_formula_contract},
};

/* Perf contracts */

long map_allocate_contract_0(std::string metric, std::vector<long> values) {
  return 0;
}
long map_get_contract_0(std::string metric, std::vector<long> values) {
  long num_traversals = values[0];
  long num_collisions = values[1];
  long success = values[2];
  long key_cached = values[3];
  long map_hash_id = values[4];
  long map_key_eq_id = values[5];
  long constant, dependency;
  assert(success);
  if (metric == "instruction count") {
    constant = 25;
  } else if (metric == "memory instructions") {
    constant = 19;
  } else if (metric == "execution cycles") {
    constant = 0 * DRAM_LATENCY + 19 * L1_LATENCY + 8;
  } else if (metric == "llvm instruction count") {
    constant = 19;
  } else if (metric == "llvm memory instructions") {
    constant = 8;
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  dependency = map_impl_get_contract(
      metric, success, key_cached, map_key_eq_id, num_traversals,
      num_collisions); /* This currently uses the NAT equality fn */

  map_hash_ptr hash_ptr = hash_ptr_map[map_hash_id];
  assert(hash_ptr);
  dependency += hash_ptr(metric);

  return constant + dependency;
}
long map_get_contract_1(std::string metric, std::vector<long> values) {
  long num_traversals = values[0];
  long num_collisions = values[1];
  long success = values[2];
  long key_cached = values[3];
  long map_hash_id = values[4];
  long map_key_eq_id = values[5];
  long constant, dependency;
  assert(success == 0);
  if (metric == "instruction count") {
    constant = 25;
  } else if (metric == "memory instructions") {
    constant = 19;
  } else if (metric == "execution cycles") {
    constant = 0 * DRAM_LATENCY + 19 * L1_LATENCY + 8;
  } else if (metric == "llvm instruction count") {
    constant = 19;
  } else if (metric == "llvm memory instructions") {
    constant = 8;
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  dependency = map_impl_get_contract(
      metric, success, key_cached, map_key_eq_id, num_traversals,
      num_collisions); /* This currently uses the NAT equality fn */

  map_hash_ptr hash_ptr = hash_ptr_map[map_hash_id];
  assert(hash_ptr);
  dependency += hash_ptr(metric);

  return constant + dependency;
}

long map_put_contract_0(std::string metric, std::vector<long> values) {
  long num_traversals = values[0];
  long key_cached = values[1];
  long constant, dependency;
  long map_hash_id = values[2];
  if (metric == "instruction count") {
    constant = 26;
  } else if (metric == "memory instructions") {
    constant = 19;
  } else if (metric == "execution cycles") {
    constant = 0 * DRAM_LATENCY + 19 * L1_LATENCY + 9;
  } else if (metric == "llvm instruction count") {
    constant = 21;
  } else if (metric == "llvm memory instructions") {
    constant = 9;
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  dependency = map_impl_put_contract(metric, key_cached, num_traversals);

  map_hash_ptr hash_ptr = hash_ptr_map[map_hash_id];
  assert(hash_ptr);
  dependency += hash_ptr(metric);
  return constant + dependency;
}

long map_erase_contract_0(std::string metric, std::vector<long> values) {
  long num_traversals = values[0];
  long num_collisions = values[1];
  long key_cached = values[2];
  long map_hash_id = values[3];
  long map_key_eq_id = values[4];
  long constant, dependency;
  if (metric == "instruction count") {
    constant = 26;
  } else if (metric == "memory instructions") {
    constant = 19;
  } else if (metric == "execution cycles") {
    constant = 0 * DRAM_LATENCY + 19 * L1_LATENCY + 9;
  } else if (metric == "llvm instruction count") {
    constant = 21;
  } else if (metric == "llvm memory instructions") {
    constant = 9;
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  dependency = map_impl_erase_contract(metric, key_cached, map_key_eq_id,
                                       num_traversals, num_collisions);

  map_hash_ptr hash_ptr = hash_ptr_map[map_hash_id];
  assert(hash_ptr);
  dependency += hash_ptr(metric);

  return constant + dependency;
}

/* Cstate contracts */

std::map<std::string, std::set<int>>
map_allocate_cstate_contract_0(std::vector<long> values) {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}
std::map<std::string, std::set<int>>
map_get_cstate_contract_0(std::vector<long> values) {

  long num_traversals = values[0];
  long num_collisions = values[1];
  long success = values[2];
  long key_cached = values[3];
  (void)key_cached;
  long map_hash_id = values[4];
  long map_key_eq_id = values[5];
  assert(success);
  std::map<std::string, std::set<int>> cstate;

  cstate["rsp"] = {-8, -16, -24, -32, -40, -48, -56, -64};
  cstate["rbx"] = {40, 24, 16, 8, 32, 0};

  std::map<std::string, int> dependency_calls;
  dependency_calls["rsp"] = -64;
  dependency_calls["rbx"] = 0;
  cstate = add_cstate_dependency(
      cstate, dependency_calls,
      map_impl_get_cstate_contract(success, map_key_eq_id, num_traversals,
                                   num_collisions));
  dependency_calls["rsp"] = -32;

  helper_cstate_fn_ptr hash_cstate = hash_cstate_ptr_map[map_hash_id];
  assert(hash_cstate && "Unknown map hash function");
  cstate = add_cstate_dependency(cstate, dependency_calls, hash_cstate());

  return cstate;
}
std::map<std::string, std::set<int>>
map_get_cstate_contract_1(std::vector<long> values) {

  long num_traversals = values[0];
  long num_collisions = values[1];
  long success = values[2];
  long key_cached = values[3];
  (void)key_cached;
  long map_hash_id = values[4];
  long map_key_eq_id = values[5];

  assert(success == 0);
  std::map<std::string, std::set<int>> cstate;

  cstate["rsp"] = {-8, -16, -24, -32, -40, -48, -56, -64};
  cstate["rbx"] = {40, 24, 16, 8, 32, 0};
  std::map<std::string, int> dependency_calls;
  dependency_calls["rsp"] = -64;
  dependency_calls["rbx"] = 0;
  cstate = add_cstate_dependency(
      cstate, dependency_calls,
      map_impl_get_cstate_contract(success, map_key_eq_id, num_traversals,
                                   num_collisions));
  dependency_calls["rsp"] = -32;
  helper_cstate_fn_ptr hash_cstate = hash_cstate_ptr_map[map_hash_id];
  assert(hash_cstate && "Unknown map hash function");
  cstate = add_cstate_dependency(cstate, dependency_calls, hash_cstate());

  return cstate;
}

std::map<std::string, std::set<int>>
map_put_cstate_contract_0(std::vector<long> values) {

  long num_traversals = values[0];
  long key_cached = values[1];
  (void)key_cached;
  long map_hash_id = values[2];
  std::map<std::string, std::set<int>> cstate;

  cstate["rsp"] = {-8, -16, -24, -32, -40, -48, -56, -64};
  cstate["rbx"] = {40, 24, 16, 8, 32, 0, 44};
  std::map<std::string, int> dependency_calls;
  dependency_calls["rsp"] = -64;
  dependency_calls["rbx"] = 0;
  cstate = add_cstate_dependency(cstate, dependency_calls,
                                 map_impl_put_cstate_contract(num_traversals));
  dependency_calls["rsp"] = -32;
  helper_cstate_fn_ptr hash_cstate = hash_cstate_ptr_map[map_hash_id];
  assert(hash_cstate && "Unknown map hash function");
  cstate = add_cstate_dependency(cstate, dependency_calls, hash_cstate());

  return cstate;
}

std::map<std::string, std::set<int>>
map_erase_cstate_contract_0(std::vector<long> values) {
  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

/* Perf Formula contracts */

perf_formula map_allocate_formula_contract_0(std::string metric,
                                             std::vector<long> values,
                                             PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = 0;
  return formula;
}

perf_formula map_get_formula_contract_0(std::string metric,
                                        std::vector<long> values,
                                        PCVAbstraction PCVAbs) {
  perf_formula formula;

  if (PCVAbs == LOOP_CTRS) {
    long num_traversals = values[0];
    long num_collisions = values[1];
    long success = values[2];
    long key_cached = values[3];
    long map_hash_id = values[4];
    long map_key_eq_id = values[5];
    long constant;
    if (metric == "instruction count") {
      constant = 25;
    } else if (metric == "memory instructions") {
      constant = 19;
    } else if (metric == "execution cycles") {
      constant = 0 * DRAM_LATENCY + 19 * L1_LATENCY + 8;
    } else if (metric == "llvm instruction count") {
      constant = 19;
    } else if (metric == "llvm memory instructions") {
      constant = 8;
    }
    else {
      assert( 0 && "Contract does not support this metric");
    }
    formula["constant"] = constant;

    formula = add_perf_formula(formula,
                               map_impl_get_formula_contract(
                                   metric, success, key_cached, map_key_eq_id,
                                   num_traversals, num_collisions, PCVAbs),
                               PCVAbs);

    map_hash_formula_ptr hash_ptr = hash_formula_ptr_map[map_hash_id];
    assert(hash_ptr);
    formula = add_perf_formula(formula, hash_ptr(metric, PCVAbs), PCVAbs);
  } else if (PCVAbs == FN_CALLS) {
    std::string fn_name = "map_get";
    std::string suffix = values[2] ? "success" : "failure";
    formula[fn_name + "_" + suffix] = 1;
  }
  return formula;
}

perf_formula map_put_formula_contract_0(std::string metric,
                                        std::vector<long> values,
                                        PCVAbstraction PCVAbs) {
  perf_formula formula;

  if (PCVAbs == LOOP_CTRS) {
    long num_traversals = values[0];
    long key_cached = values[1];
    long constant;
    long map_hash_id = values[2];
    if (metric == "instruction count") {
      constant = 26;
    } else if (metric == "memory instructions") {
      constant = 19;
    } else if (metric == "execution cycles") {
      constant = 0 * DRAM_LATENCY + 19 * L1_LATENCY + 9;
    } else if (metric == "llvm instruction count") {
      constant = 21;
    } else if (metric == "llvm memory instructions") {
      constant = 9;
    }
    else {
      assert( 0 && "Contract does not support this metric");
    }
    formula["constant"] = constant;
    formula = add_perf_formula(formula,
                               map_impl_put_formula_contract(
                                   metric, key_cached, num_traversals, PCVAbs),
                               PCVAbs);
    map_hash_formula_ptr hash_ptr = hash_formula_ptr_map[map_hash_id];
    assert(hash_ptr);
    formula = add_perf_formula(formula, hash_ptr(metric, PCVAbs), PCVAbs);
  } else if (PCVAbs == FN_CALLS) {
    formula["map_put"] = 1;
  }
  return formula;
}

perf_formula map_erase_formula_contract_0(std::string metric,
                                          std::vector<long> values,
                                          PCVAbstraction PCVAbs) {
  perf_formula formula;

  if (PCVAbs == LOOP_CTRS) {
    long num_traversals = values[0];
    long num_collisions = values[1];
    long key_cached = values[2];
    long map_hash_id = values[3];
    long map_key_eq_id = values[4];
    long constant;
    if (metric == "instruction count") {
      constant = 26;
    } else if (metric == "memory instructions") {
      constant = 19;
    } else if (metric == "execution cycles") {
      constant = 0 * DRAM_LATENCY + 19 * L1_LATENCY + 9;
    }
    else if (metric == "llvm instruction count") {
      constant = 21;
    } else if (metric == "llvm memory instructions") {
      constant = 9;
    }
    else {
      assert( 0 && "Contract does not support this metric");
    }
    formula["constant"] = constant;
    formula = add_perf_formula(
        formula,
        map_impl_erase_formula_contract(metric, key_cached, map_key_eq_id,
                                        num_traversals, num_collisions, PCVAbs),
        PCVAbs);
    map_hash_formula_ptr hash_ptr = hash_formula_ptr_map[map_hash_id];
    assert(hash_ptr);
    formula = add_perf_formula(formula, hash_ptr(metric, PCVAbs), PCVAbs);
  } else if (PCVAbs == FN_CALLS) {
    formula["map_erase"] = 1;
  }
  return formula;
}