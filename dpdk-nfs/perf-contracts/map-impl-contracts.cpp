#include "map-impl-contracts.h"

std::map<long, helper_cstate_fn_ptr> eq_cstate_ptr_map = {
    {1, &flow_id_eq_cstate_contract},
    {2, &ether_addr_eq_cstate_contract},
    {3, &lb_flow_equality_cstate_contract},
    {4, &lb_ip_equality_cstate_contract},
    {5, &policer_flow_eq_cstate_contract},
};

std::map<long, map_key_eq_formula_ptr> key_eq_formula_map = {
    {1, &flow_id_eq_formula_contract},
    {2, &ether_addr_eq_formula_contract},
    {3, &lb_flow_equality_formula_contract},
    {4, &lb_ip_equality_formula_contract},
    {5, &policer_flow_eq_formula_contract},
};

/* Perf contracts */

std::map<long, map_key_eq_ptr> key_eq_map = {
    {1, &flow_id_eq_contract},
    {2, &ether_addr_eq_contract},
    {3, &lb_flow_equality_contract},
    {4, &lb_ip_equality_contract},
    {5, &policer_flow_eq_contract}
};

long map_impl_init_contract(std::string metric, long success, long capacity) {
  if (success) {
    return (2 * capacity + 1) *
           DRAM_LATENCY; // Have not gone through patterns here. In progress
  } else
    return 0;
}

long map_impl_put_contract(std::string metric, long recent,
                           long num_traversals) {
  long constant, dynamic;
  if (metric == "instruction count") {
    dynamic = 0;
    if (num_traversals == 1) {
      constant = 52; // (9+15+31)
    } else {
      constant = 51; // 9+15+31
      dynamic = 13 * (num_traversals - 1); // 15+25
    }
  } else if (metric == "memory instructions") {
    dynamic = 0;
    if (num_traversals == 1) {
      constant = 25;
    } else {
      constant = 24;
      dynamic = 2 * (num_traversals - 1);
    }
  } else if (metric == "execution cycles") {
    constant = 16 * L1_LATENCY + 32;
    if (recent)
      constant += 4 * L1_LATENCY;
    else
      constant += 4 * DRAM_LATENCY;

    dynamic = 0;
    if (num_traversals > 1) {
      num_traversals--;
      dynamic = (2 * num_traversals) * L1_LATENCY + 18 * num_traversals;
    }
  } else if (metric == "llvm instruction count") {
    constant = 27;
    dynamic = 17 * (num_traversals - 1);
  } else if (metric == "llvm memory instructions") {
    constant = 5;
    dynamic = 3 * (num_traversals - 1);
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant + dynamic;
}

long map_impl_get_contract(std::string metric, long success, long recent,
                           long map_key_eq, long num_traversals,
                           long num_collisions) {
  map_key_eq_ptr eq_ptr = key_eq_map[map_key_eq];
  assert(eq_ptr && "Invalid map equality function");
  long constant, dynamic;
  constant = 0;
  dynamic = 0;
  long rem_traversals;
  if (metric == "instruction count") {
    if (success) {
      if (num_traversals == 1) {
        constant = 65 + eq_ptr(metric, success); 
      } else {
        constant = 65 + eq_ptr(metric, success); // 10 16 30 34 41 43
        dynamic += num_collisions * (28 + eq_ptr(metric, 0)); // 16 30 34 38
        rem_traversals = num_traversals - num_collisions - 1;
        dynamic += rem_traversals * 18; // 16 30 36 38
      }
    } 
    else {
      constant = 52;
      dynamic += num_collisions * (28 + eq_ptr(metric, 0)); //
      rem_traversals = num_traversals - num_collisions - 1;
      dynamic += rem_traversals * 18;
    }
  } else if (metric == "memory instructions") {
    if (success) {
      if (num_traversals == 1) {
        constant = 37 + eq_ptr(metric, success);
      } else {
        constant = 37 + eq_ptr(metric, success);
        dynamic += num_collisions * (14 + eq_ptr(metric, 0));
        rem_traversals = num_traversals - num_collisions - 1;
        dynamic += rem_traversals * 4;
      }
    } 
    else{
      constant = 28;
      dynamic += num_collisions * (14 + eq_ptr(metric, 0));
      rem_traversals = num_traversals - num_collisions - 1;
      dynamic += rem_traversals * 4;
    }
  } else if (metric == "execution cycles") {
    if (success) {
      if (recent)
        constant = 4 * L1_LATENCY;
      else
        constant = 4 * DRAM_LATENCY;
      constant += 33 * L1_LATENCY + 31 + eq_ptr(metric, success);
    } else {
      if (recent)
        constant = 2 * L1_LATENCY;
      else
        constant = 2 * DRAM_LATENCY;
      constant += 20 * L1_LATENCY + 32;
    }
    dynamic = 0;
    if (num_traversals > 1) {
      num_traversals--;
      dynamic = (3 * num_traversals) * L1_LATENCY + 18 * num_traversals;
    }
    if (num_collisions > 0) {
      dynamic += 1 * DRAM_LATENCY + (1 * num_collisions) * L1_LATENCY +
                 (24 * num_collisions) * L1_LATENCY +
                 15 * num_collisions; // Keys are 16B, so always cache aligned
    }
  } else if (metric == "llvm instruction count") {
    if(success) {
      constant = 32 + eq_ptr(metric, success);
      dynamic += num_collisions * (23 + eq_ptr(metric, 0)); // 16 30 34 38
      rem_traversals = num_traversals - num_collisions - 1;
      dynamic += rem_traversals * 23; // 16 30 36 38
    } else {
      constant = 28; // 10 16 30 36 46
      dynamic += num_collisions * (23 + eq_ptr(metric, 0)); // 16 30 34 38
      rem_traversals = num_traversals - num_collisions - 1;
      dynamic += rem_traversals * 23; // 16 30 36 38
    }
  } else if (metric == "llvm memory instructions") {
    if(success) {
      constant = 6 + eq_ptr(metric, success);
      dynamic += num_collisions * (4 + eq_ptr(metric, 0)); // 16 30 34 38
      rem_traversals = num_traversals - num_collisions - 1;
      dynamic += rem_traversals * 4; // 16 30 36 38
    } else {
      constant = 4; // 10 16 30 36 46
      dynamic += num_collisions * (4 + eq_ptr(metric, 0)); // 16 30 34 38
      rem_traversals = num_traversals - num_collisions - 1;
      dynamic += rem_traversals * 4; // 16 30 36 38
    }
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }

  return constant + dynamic;
}

long map_impl_erase_contract(std::string metric, long recent, long map_key_eq,
                             long num_traversals, long num_collisions) {
  map_key_eq_ptr eq_ptr = key_eq_map[map_key_eq];
  assert(eq_ptr && "Invalid map equality function");
  long constant, dynamic;
  constant = 0;
  dynamic = 0;
  long rem_traversals;
  if (metric == "instruction count") {
    if (num_traversals == 1) {
      constant = 69 + eq_ptr(metric, 1); //
    } else {
      constant = 69 + eq_ptr(metric, 1); // 9 15 29 33 35 45
      rem_traversals = num_traversals - num_collisions - 1;
      dynamic += num_collisions * (39 + eq_ptr(metric, 0)); // 15 29 33 41
      dynamic += 19 * rem_traversals; // 15 29 41
    }
  } else if (metric == "memory instructions") {
    if (num_traversals == 1) {
      constant = 43 + eq_ptr(metric, 1);
    } else {
      constant = 43 + eq_ptr(metric, 1);
      rem_traversals = num_traversals - num_collisions - 1;
      dynamic += num_collisions * (21 + eq_ptr(metric, 0));
      dynamic += 4 * rem_traversals;
    }
  } else if (metric == "execution cycles") {
    constant = 39 * L1_LATENCY + 31 + eq_ptr(metric, 1);
    if (recent)
      constant += 4 * L1_LATENCY;
    else
      constant += 4 * DRAM_LATENCY;
    if (num_traversals > 1) {
      num_traversals--;
      dynamic = (3 * num_traversals) * L1_LATENCY + 17 * num_traversals;
    }
    if (num_collisions > 0) {
      dynamic += 1 * DRAM_LATENCY + (1 * num_collisions) * L1_LATENCY +
                 (24 * num_collisions) * L1_LATENCY +
                 15 * num_collisions; // Keys are 16B, so always cache aligned
    }
  } else if (metric == "llvm instruction count") {
      constant = 35 + eq_ptr(metric, 1); // 9 15 29 33 35 45
      rem_traversals = num_traversals - num_collisions - 1;
      dynamic += num_collisions * (25 + eq_ptr(metric, 0)); // 15 29 33 41
      dynamic += 23 * rem_traversals; // 15 29 41
  } else if (metric == "llvm memory instructions") {
      constant = 7 + eq_ptr(metric, 1); // 9 15 29 33 35 45
      rem_traversals = num_traversals - num_collisions - 1;
      dynamic += num_collisions * (5 + eq_ptr(metric, 0)); // 15 29 33 41
      dynamic += 5 * rem_traversals; // 15 29 41
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant + dynamic;
}

/* Cstate contracts */

std::map<std::string, std::set<int>>
map_impl_init_cstate_contract(long success, long capacity) {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>>
map_impl_put_cstate_contract(long num_traversals) {

  std::map<std::string, std::set<int>> cstate;
  cstate["rsp"] = {-8, -16, -24, -32, -40, 32, 16, 24};
  return cstate;
}

std::map<std::string, std::set<int>>
map_impl_get_cstate_contract(long success, long map_key_eq, long num_traversals,
                             long num_collisions) {

  std::map<std::string, std::set<int>> cstate;
  if (success) {
    cstate["rsp"] = {-8,  -16, -24, -32, -40, 32,   16,  -80, -64, -56,
                     -80, -84, -88, -96, -72, -104, -64, -48, 24};
    std::map<std::string, int> dependency_calls;
    dependency_calls["rsp"] = -104;
    helper_cstate_fn_ptr eq_cstate = eq_cstate_ptr_map[map_key_eq];
    assert(eq_cstate && "Unknown map hash function");
    cstate = add_cstate_dependency(cstate, dependency_calls, eq_cstate());
  } else {
    cstate["rsp"] = {-8, -16, -24, -32, -40, 32, 16, -80, -64, -56};
  }
  return cstate;
}

std::map<std::string, std::set<int>>
map_impl_erase_cstate_contract(long map_key_eq, long num_traversals,
                               long num_collisions) {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

/* Perf Formula contracts */

perf_formula map_impl_init_formula_contract(std::string metric, long success,
                                            long capacity,
                                            PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = 0;
  return formula;
}

perf_formula map_impl_put_formula_contract(std::string metric, long recent,
                                           long num_traversals,
                                           PCVAbstraction PCVAbs) {
  perf_formula formula;

  if (PCVAbs == LOOP_CTRS) {
    long constant, traversals_coeff;
    if (metric == "instruction count") {
      constant = 39;
      traversals_coeff = 13;
    } else if (metric == "memory instructions") {
      constant = 23;
      traversals_coeff = 2;
    } else if (metric == "execution cycles") {
      constant = 16 * L1_LATENCY + 32;
      if (recent)
        constant += 4 * L1_LATENCY;
      else
        constant += 4 * DRAM_LATENCY;
      traversals_coeff = 2 * L1_LATENCY + 18;
    } else if (metric == "llvm instruction count"){
      constant = 10;
      traversals_coeff = 10;
    } else if(metric == "llvm memory instructions"){
      constant = 2;
      traversals_coeff = 3;
    }
    else {
      assert( 0 && "Contract does not support this metric");
    }
    formula["constant"] = constant;
    formula["t"] = traversals_coeff;
  } else if (PCVAbs == FN_CALLS) {
    assert(0 && "Internal function should never be called");
  }
  return formula;
}

perf_formula map_impl_get_formula_contract(std::string metric, long success,
                                           long recent, long map_key_eq,
                                           long num_traversals,
                                           long num_collisions,
                                           PCVAbstraction PCVAbs) {
  perf_formula formula;

  if (PCVAbs == LOOP_CTRS) {
    long constant, traversals_coeff, collisions_coeff;
    if (metric == "instruction count") {
      if (success) {
        constant = 47;
      } else {
        constant = 34;
      }
      traversals_coeff = 18;
      collisions_coeff = 10;
    } else if (metric == "memory instructions") {
      if (success) {
        constant = 33;
      } else {
        constant = 24;
      }
      traversals_coeff = 4;
      collisions_coeff = 10;
    } else if (metric == "execution cycles") {
      if (success) {
        if (recent)
          constant = 4 * L1_LATENCY;
        else
          constant = 4 * DRAM_LATENCY;
        constant += 30 * L1_LATENCY + 13;
      } else {
        if (recent)
          constant = 4 * L1_LATENCY;
        else
          constant = 4 * DRAM_LATENCY;
        constant += 17 * L1_LATENCY + 14;
      }
      traversals_coeff = 3 * L1_LATENCY + 18;
      collisions_coeff = 25 * L1_LATENCY + 15;
    } else if (metric == "llvm instruction count") {
      if(success){
        constant = 9;
      }
      else {
        constant = 5;
      }
      traversals_coeff = 23;
      collisions_coeff = 0;
    } else if(metric == "llvm memory instructions") {
      if(success){
        constant = 2;
      }
      else{
        constant = 0;
      }
      traversals_coeff = 4;
      collisions_coeff = 0;
    }
    else {
      assert( 0 && "Contract does not support this metric");
    }

    formula["constant"] = constant;
    formula["t"] = traversals_coeff;
    formula["c"] = collisions_coeff;

    map_key_eq_formula_ptr eq_ptr = key_eq_formula_map[map_key_eq];
    assert(eq_ptr && "Invalid map equality function");

    perf_formula constant_dependency;
    if (success)
      constant_dependency = eq_ptr(metric, success, PCVAbs);
    else
      constant_dependency["constant"] = 0;
    perf_formula dynamic_dependency;
    dynamic_dependency["c"] = 1;
    dynamic_dependency = multiply_perf_formula(
        dynamic_dependency, eq_ptr(metric, 0, PCVAbs), PCVAbs);

    formula = add_perf_formula(formula, constant_dependency, PCVAbs);
    formula = add_perf_formula(formula, dynamic_dependency, PCVAbs);
  } else if (PCVAbs == FN_CALLS) {
    assert(0 && "Internal function should never be called");
  }

  return formula;
}

perf_formula map_impl_erase_formula_contract(std::string metric, long recent,
                                             long map_key_eq,
                                             long num_traversals,
                                             long num_collisions,
                                             PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS) {
    long constant, traversals_coeff, collisions_coeff;
    if (metric == "instruction count") {
      constant = 50;
      traversals_coeff = 19;
      collisions_coeff = 20;
    } else if (metric == "memory instructions") {
      constant = 39;
      traversals_coeff = 4;
      collisions_coeff = 17;
    } else if (metric == "execution cycles") {
      if (recent)
        constant = 4 * L1_LATENCY;
      else
        constant = 4 * DRAM_LATENCY;
      constant += 36 * L1_LATENCY + 14;
      traversals_coeff = 3 * L1_LATENCY + 17;
      collisions_coeff = 25 * L1_LATENCY + 15;
    } else if (metric == "llvm instruction count"){
      constant = 12;
      traversals_coeff = 23;
      collisions_coeff = 2;
    } else if (metric == "llvm memory instructions"){
      constant = 2;
      traversals_coeff = 5;
      collisions_coeff = 0;
    }
    else {
      assert( 0 && "Contract does not support this metric");
    }

    formula["constant"] = constant;
    formula["t"] = traversals_coeff;
    formula["c"] = collisions_coeff;

    map_key_eq_formula_ptr eq_ptr = key_eq_formula_map[map_key_eq];
    assert(eq_ptr && "Invalid map equality function");

    perf_formula constant_dependency = eq_ptr(metric, 1,PCVAbs);

    perf_formula dynamic_dependency;
    dynamic_dependency["c"] = 1;
    dynamic_dependency =
        multiply_perf_formula(dynamic_dependency, eq_ptr(metric, 0,PCVAbs),PCVAbs);

    formula = add_perf_formula(formula, constant_dependency,PCVAbs);
    formula = add_perf_formula(formula, dynamic_dependency,PCVAbs);
  }

  else if (PCVAbs == FN_CALLS) {
    assert(0 && "Internal function should never be called");
  }

  return formula;
}