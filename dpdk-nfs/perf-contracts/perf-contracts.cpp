#include "klee/perf-contracts.h"

/* Subcontract files */
#include "cht-contracts.h"
#include "contract-params.h"
#include "dchain-contracts.h"
#include "dpdk-contracts.h"
#include "expirator-contracts.h"
#include "helper-contracts.h"
#include "ip-opt-contracts.h"
#include "lpm-contracts.h"
#include "map-contracts.h"
#include "map-impl-contracts.h"
#include "vector-contracts.h"
#include "natasha-contracts.h"
#include "bpf-map-contracts.h"

std::vector<std::string> supported_metrics;
std::vector<std::string> fn_names;
std::map<std::string, std::string> user_variables;
std::map<std::string, std::set<std::string>> optimization_variables;
std::map<std::string, std::map<int, std::string>> constraints;
std::map<std::string, std::map<int, perf_calc_fn_ptr>> perf_fn_ptrs;
std::map<std::string, std::map<int, cstate_fn_ptr>> cstate_fn_ptrs;
std::map<std::string, std::map<int, perf_formula_ptr>> perf_formula_fn_ptrs;
std::map<std::string, std::map<int, std::string>>
    perf_formula_fn_names; /*For higher PCV abstraction level */

std::map<PCVAbstraction, std::vector<std::string>> supported_pcv_symbols;
std::map<PCVAbstraction, std::string> pcvabs_prefix;

std::map<std::string, std::vector<std::string>> fn_user_variables;
std::map<std::string, std::vector<std::string>> fn_optimization_variables;
std::map<std::string, std::vector<std::string>> fn_variables;
std::vector<std::string> relevant_vars;
std::vector<long> relevant_vals;

/* Contract stitcher for concrete state */
std::map<std::string, std::set<int>>
add_cstate_dependency(std::map<std::string, std::set<int>> caller,
                      std::map<std::string, int> caller_state,
                      std::map<std::string, std::set<int>> callee) {

  std::map<std::string, std::set<int>> temp;
  for (auto it : caller_state) {
    if (callee.find(it.first) != callee.end()) {
      for (auto it1 : callee[it.first]) {
        temp[it.first].insert(it1 + it.second);
      }
    }
  }

  for (auto it : caller) {
    std::set<int> tempset;
    std::set_union(std::begin(it.second), std::end(it.second),
                   std::begin(temp[it.first]), std::end(temp[it.first]),
                   std::inserter(tempset, std::begin(tempset)));
    caller[it.first] = tempset;
  }

  return caller;
}

/* Functions called by workflow  */

bool check_metric(std::string metric) {
  if (std::find(supported_metrics.begin(), supported_metrics.end(), metric) !=
      supported_metrics.end()) {
    return true;
  }
  return false;
}

bool check_formula(perf_formula formula, PCVAbstraction PCVAbs) {
  for (auto itf = formula.begin(); itf != formula.end(); ++itf) {
    if (std::find(supported_pcv_symbols[PCVAbs].begin(),
                  supported_pcv_symbols[PCVAbs].end(), itf->first) !=
        supported_pcv_symbols[PCVAbs].end()) /* PCV present directly */ {
      continue;
    } else {
      if (PCVAbs == LOOP_CTRS) {
        for (unsigned i = 0; i < itf->first.length();
             i++) { /* Possible multiplication of two PCVs */
          std::string search_str(1, itf->first[i]);
          if (std::find(supported_pcv_symbols[PCVAbs].begin(),
                        supported_pcv_symbols[PCVAbs].end(),
                        search_str) == supported_pcv_symbols[PCVAbs].end())
            return false;
        }
      } else if (PCVAbs == FN_CALLS)
        return false;
    }
  }
  return true;
}

perf_formula add_perf_formula(perf_formula ip1, perf_formula ip2,
                              PCVAbstraction PCVAbs) {
  assert(check_formula(ip1, PCVAbs) && check_formula(ip2, PCVAbs) &&
         "Bad input formulae");
  perf_formula result = ip1;
  for (auto it = ip2.begin(); it != ip2.end(); it++) {
    if (result[it->first])
      result[it->first] += it->second;
    else
      result[it->first] = it->second;
  }
  return result;
}

perf_formula multiply_perf_formula(perf_formula ip1, perf_formula ip2,
                                   PCVAbstraction PCVAbs) {

  /** This is a simple first implementation that makes the following
  assumptions:
  * PCV Abstraction level == LOOP_CTRS
  * The PCVs in ip1 and ip2 are disjoint, the only common PCV is the constant
  * This assumption holds for all DS used in the NSDI'19 paper
  * Because of this assumption, we can naively multiply term by term and add
  them all together
  * Similar terms in multiplication only arise when the same PCVs exist on both
  sides */
  assert(PCVAbs == LOOP_CTRS && "Cannot multiply higher abstraction levels");
  assert(check_formula(ip1, PCVAbs) && check_formula(ip2, PCVAbs) &&
         "Bad input formulae");
  perf_formula result;
  std::string term1, term2;
  for (auto it1 : ip1) {
    term1 = it1.first == "constant" ? "" : it1.first;
    for (auto it2 : ip2) {
      if (term1 == "")
        term2 = it2.first;
      else
        term2 = it2.first == "constant" ? "" : it2.first;
      result[term1 + term2] = it1.second * it2.second;
    }
  }
  return result;
}

std::string display_perf_formula(perf_formula formula, PCVAbstraction PCVAbs) {

  assert(check_formula(formula, PCVAbs) && "Invalid Formula provided");
  std::string f = "";
  for (auto it : formula) {

    if (f != "")
      f += "+ ";
    if (std::find(supported_pcv_symbols[PCVAbs].begin(),
                  supported_pcv_symbols[PCVAbs].end(), it.first) !=
        supported_pcv_symbols[PCVAbs].end()) /* PCV present directly */ {
      f = f + std::to_string(it.second);
      if (it.first != "constant")
        f += "*" + it.first;
      f += " ";
    } else {

      f = f + std::to_string(it.second);
      for (unsigned i = 0; i < it.first.length();
           i++) { /* Multiplication of two PCVs */
        std::string pcv(1, it.first[i]);
        f = f + "*" + pcv;
      }
      f += " ";
    }
  }
  f += "\n";
  return f;
}

bool get_relevant_vars(std::string function_name,
                       std::map<std::string, long> variables) {
  relevant_vars = fn_variables[function_name];
  relevant_vals.clear();
  for (std::vector<std::string>::iterator i = relevant_vars.begin();
       i != relevant_vars.end(); ++i) {
    if (variables.find(*i) == variables.end()) {
      std::cerr << "Required variable " << *i << " not sent for function "
                << function_name << std::endl;
      return false;
    } else
      relevant_vals.insert(relevant_vals.end(), variables[*i]);
  }
  return true;
}

void contract_init() {

#ifdef METRICS_X86
  supported_metrics = {"execution cycles", "instruction count",
                       "memory instructions"};
#else
  supported_metrics = {"llvm instruction count", "llvm memory instructions"};
#endif
  fn_names = {
      "dchain_allocate",
      "dchain_allocate_new_index",
      "dchain_rejuvenate_index",
      "dchain_is_index_allocated",
      "map_allocate",
      "map_get",
      "map_put",
      "map_erase",
      "expire_items_single_map",
      "vector_allocate",
      "vector_borrow",
      "vector_return",
      "handle_packet_timestamp",
      "lpm_init",
      "lpm_lookup",
      "trace_reset_buffers",
      "lb_find_preferred_available_backend",
      "nf_set_ipv4_checksum",
      "flood", /*Hack*/
      "process_ip_packet",
      "bpf_map_lookup_elem",
      "bpf_map_update_elem",
  };
  /* List of variables the user can set */
  user_variables = {
      {"map_occupancy",
       "(Sub w32 (ReadLSB w32 0 initial_map_capacity) (w32 1))"},
      {"Num_bucket_traversals", "(ReadLSB w32 0 initial_map_occupancy)"},
      {"Num_hash_collisions", "(ReadLSB w32 0 initial_Num_bucket_traversals)"},
      {"expired_flows", "(ReadLSB w32 0 initial_map_occupancy)"},
      {"available_backends", "(ReadLSB w32 0 initial_backend_capacity)"},
      {"lpm_stages", "(ReadLSB w32 0 initial_max_lpm_depth)"},
  };

  supported_pcv_symbols = {
      {LOOP_CTRS,
       {
           "e",        /* expired flows */
           "t",        /* bucket traversals */
           "c",        /* hash collisions */
           "s",        /* lpm stages */
           "constant", /* final constant*/
       }},
      {FN_CALLS, {}},
  };

  pcvabs_prefix = {
      {LOOP_CTRS, "LOOP_CTRS"},
      {FN_CALLS, "FN_CALLS"},
  };

  /* List of shadow variables that allow more precise contracts.
  The user cannot specify these unless they are also UVs*/

  optimization_variables = {
      {"dchain_out_of_space", {"(w32 0)", "(w32 1)"}},
      {"map_has_this_key", {"(w32 0)", "(w32 1)"}},
      {"multi_stage_lookup", {"(w32 0)", "(w32 1)"}},
      {"recent_flow", {"(w32 0)", "(w32 1)"}},
      {"map_key_cached", {"(w32 0)", "(w32 1)"}},
      {"map_hash", {"(w64 1)", "(w64 2)", "(w64 3)", "(w64 4)", "(w64 5)"}},
      {"map_key_eq", {"(w64 1)", "(w64 2)", "(w64 3)", "(w64 4)", "(w64 5)"}},
      {"rewrite_src_ip", {"(w32 0)", "(w32 1)"}},
      {"rewrite_dst_ip", {"(w32 0)", "(w32 1)"}},
      {"matching_rule_found", {"(w32 0)", "(w32 1)"}},
      {"bpf_map_type", {"(w32 1)", "(w32 2)", "(w32 5)", "(w32 6)", "(w32 9)", "(w32 12)", "(w32 13)", "(w32 14)", "(w32 16)"}}, 
  };

  /* Map of function name to user variables */
  fn_user_variables = {
      {"map_get", {"Num_bucket_traversals", "Num_hash_collisions"}},
      {"map_put", {"Num_bucket_traversals"}},
      {"map_erase", {"Num_bucket_traversals", "Num_hash_collisions"}},
      {"expire_items_single_map",
       {"Num_bucket_traversals", "Num_hash_collisions", "expired_flows"}},
      {"lb_find_preferred_available_backend", {"available_backends"}},
      {"process_ip_packet", {"lpm_stages"}},
  };

  /* Map of function name to shadow variable. If a variable is both a UV and a
  shadow variable it must be listed in the fn_user_variables map. */
  fn_optimization_variables = {
      {"dchain_allocate_new_index", {"dchain_out_of_space"}},
      {"map_get",
       {"map_has_this_key", "map_key_cached", "map_hash", "map_key_eq"}},
      {"map_put", {"map_key_cached", "map_hash"}},
      {"map_erase", {"map_key_cached", "map_hash", "map_key_eq"}},
      {"expire_items_single_map", {"map_hash", "map_key_eq"}},
      {"lpm_lookup", {"multi_stage_lookup"}},
      {"process_ip_packet", {"rewrite_src_ip", "rewrite_dst_ip", "matching_rule_found"}},
      {"bpf_map_lookup_elem",{"bpf_map_type"}},
      {"bpf_map_update_elem",{"bpf_map_type"}},
  };

  fn_variables.insert(fn_user_variables.begin(), fn_user_variables.end());
  for (std::map<std::string, std::vector<std::string>>::iterator i =
           fn_optimization_variables.begin();
       i != fn_optimization_variables.end(); ++i) {
    if (fn_variables.find(i->first) == fn_variables.end()) {
      fn_variables[i->first] = i->second;
    } else {
      fn_variables[i->first].insert(fn_variables[i->first].end(),
                                    i->second.begin(), i->second.end());
    }
  }
  constraints = {
      //{"fn_name",{ {0,{"constraint1","constraint2" }} , {1,{"constraint1",
      //"constraint2"}}  }},
      {"dchain_allocate", {{0, "true"}}},
      {"dchain_allocate_new_index",
       {{0, "(Eq 0 (ReadLSB w32 0 current_dchain_out_of_space))"},
        {1, "(Eq false (Eq 0 (ReadLSB w32 0 current_dchain_out_of_space)))"}}},
      {"dchain_rejuvenate_index", {{0, "true"}}},
      {"dchain_is_index_allocated", {{0, "true"}}},
      {"map_allocate", {{0, "true"}}},
      {"map_get",
       {{0, "(Eq false (Eq 0 (ReadLSB w32 0 current_map_has_this_key)))"},
        {1, "(Eq 0 (ReadLSB w32 0 current_map_has_this_key))"}}},
      {"map_put", {{0, "true"}}},
      {"map_erase", {{0, "true"}}},
      {"expire_items_single_map", {{0, "true"}}},
      {"vector_allocate", {{0, "true"}}},
      {"vector_borrow", {{0, "true"}}},
      {"vector_return", {{0, "true"}}},
      {"handle_packet_timestamp", {{0, "true"}}},
      {"lpm_init", {{0, "true"}}},
      {"lpm_lookup", {{0, "true"}}},
      {"trace_reset_buffers", {{0, "true"}}},
      {"lb_find_preferred_available_backend", {{0, "true"}}},
      {"flood", {{0, "true"}}},
      {"nf_set_ipv4_checksum", {{0, "true"}}},  
      {"process_ip_packet", {{0, "true"}}},    
      {"bpf_map_lookup_elem", {{0, "true"}}},   
      {"bpf_map_update_elem", {{0, "true"}}},     
  };

  perf_fn_ptrs = {
      {"dchain_allocate", {{0, &dchain_allocate_contract_0}}},
      {"dchain_allocate_new_index",
       {{0, &dchain_allocate_new_index_contract_0},
        {1, &dchain_allocate_new_index_contract_1}}},
      {"dchain_rejuvenate_index", {{0, &dchain_rejuvenate_index_contract_0}}},
      {"dchain_is_index_allocated",
       {{0, &dchain_is_index_allocated_contract_0}}},
      {"map_allocate", {{0, &map_allocate_contract_0}}},
      {"map_get", {{0, &map_get_contract_0}, {1, &map_get_contract_1}}},
      {"map_put", {{0, &map_put_contract_0}}},
      {"map_erase", {{0, &map_erase_contract_0}}},
      {"expire_items_single_map", {{0, &expire_items_single_map_contract_0}}},
      {"vector_allocate", {{0, &vector_allocate_contract_0}}},
      {"vector_borrow", {{0, &vector_borrow_contract_0}}},
      {"vector_return", {{0, &vector_return_contract_0}}},
      {"lpm_init", {{0, &lpm_init_contract_0}}},
      {"lpm_lookup", {{0, &lpm_lookup_contract_0}}},
      {"trace_reset_buffers", {{0, &trace_reset_buffers_contract_0}}},
      {"handle_packet_timestamp", {{0, &handle_packet_timestamp_contract_0}}},
      {"lb_find_preferred_available_backend",
       {{0, &lb_find_preferred_available_backend_contract_0}}},
      {"flood", {{0, &flood_contract_0}}},
      {"nf_set_ipv4_checksum", {{0, &checksum_contract_0}}},
      {"process_ip_packet", {{0, &process_ip_packet_contract_0}}},
      {"bpf_map_lookup_elem", {{0, &bpf_map_lookup_elem_contract_0}}},
      {"bpf_map_update_elem", {{0, &bpf_map_update_elem_contract_0}}},
  };

  cstate_fn_ptrs = {
      {"dchain_allocate", {{0, &dchain_allocate_cstate_contract_0}}},
      {"dchain_allocate_new_index",
       {{0, &dchain_allocate_new_index_cstate_contract_0},
        {1, &dchain_allocate_new_index_cstate_contract_1}}},
      {"dchain_rejuvenate_index",
       {{0, &dchain_rejuvenate_index_cstate_contract_0}}},
      {"dchain_is_index_allocated",
       {{0, &dchain_is_index_allocated_cstate_contract_0}}},
      {"map_allocate", {{0, &map_allocate_cstate_contract_0}}},
      {"map_get",
       {{0, &map_get_cstate_contract_0}, {1, &map_get_cstate_contract_1}}},
      {"map_put", {{0, &map_put_cstate_contract_0}}},
      {"map_erase", {{0, &map_erase_cstate_contract_0}}},
      {"expire_items_single_map",
       {{0, &expire_items_single_map_cstate_contract_0}}},
      {"vector_allocate", {{0, &vector_allocate_cstate_contract_0}}},
      {"vector_borrow", {{0, &vector_borrow_cstate_contract_0}}},
      {"vector_return", {{0, &vector_return_cstate_contract_0}}},
      {"lpm_init", {{0, &lpm_init_cstate_contract_0}}},
      {"lpm_lookup", {{0, &lpm_lookup_cstate_contract_0}}},
      {"trace_reset_buffers", {{0, &trace_reset_buffers_cstate_contract_0}}},
      {"handle_packet_timestamp",
       {{0, &handle_packet_timestamp_cstate_contract_0}}},
      {"lb_find_preferred_available_backend",
       {{0, &lb_find_preferred_available_backend_cstate_contract_0}}},
      {"flood", {{0, &flood_cstate_contract_0}}},
      {"nf_set_ipv4_checksum", {{0, &checksum_cstate_contract_0}}},
      {"process_ip_packet", {{0, &process_ip_packet_cstate_contract_0}}},
      {"bpf_map_lookup_elem", {{0, &bpf_map_lookup_elem_cstate_contract_0}}},
      {"bpf_map_update_elem", {{0, &bpf_map_update_elem_cstate_contract_0}}},
  };

  perf_formula_fn_ptrs = {
      {"dchain_allocate", {{0, &dchain_allocate_formula_contract_0}}},
      {"dchain_allocate_new_index",
       {{0, &dchain_allocate_new_index_formula_contract_0},
        {1, &dchain_allocate_new_index_formula_contract_1}}},
      {"dchain_rejuvenate_index",
       {{0, &dchain_rejuvenate_index_formula_contract_0}}},
      {"dchain_is_index_allocated",
       {{0, &dchain_is_index_allocated_formula_contract_0}}},
      {"map_allocate", {{0, &map_allocate_formula_contract_0}}},
      {"map_get",
       {{0, &map_get_formula_contract_0}, {1, &map_get_formula_contract_0}}},
      {"map_put", {{0, &map_put_formula_contract_0}}},
      {"map_erase", {{0, &map_erase_formula_contract_0}}},
      {"expire_items_single_map",
       {{0, &expire_items_single_map_formula_contract_0}}},
      {"vector_allocate", {{0, &vector_allocate_formula_contract_0}}},
      {"vector_borrow", {{0, &vector_borrow_formula_contract_0}}},
      {"vector_return", {{0, &vector_return_formula_contract_0}}},
      {"lpm_init", {{0, &lpm_init_formula_contract_0}}},
      {"lpm_lookup", {{0, &lpm_lookup_formula_contract_0}}},
      {"trace_reset_buffers", {{0, &trace_reset_buffers_formula_contract_0}}},
      {"handle_packet_timestamp",
       {{0, &handle_packet_timestamp_formula_contract_0}}},
      {"lb_find_preferred_available_backend",
       {{0, &lb_find_preferred_available_backend_formula_contract_0}}},
      {"flood", {{0, &flood_formula_contract_0}}},
      {"nf_set_ipv4_checksum", {{0, &checksum_formula_contract_0}}},
      {"process_ip_packet", {{0, &process_ip_packet_formula_contract_0}}},
      {"bpf_map_lookup_elem", {{0, &bpf_map_lookup_elem_formula_contract_0}}},
      {"bpf_map_update_elem", {{0, &bpf_map_update_elem_formula_contract_0}}},
  };

  perf_formula_fn_names = {
      /* Automate this */
      {"dchain_allocate", {{0, ""}}},
      {"dchain_allocate_new_index",
       {{0, "dchain_allocate_new_index_success"},
        {1, "dchain_allocate_new_index_failure"}}},
      {"dchain_rejuvenate_index", {{0, "dchain_rejuvenate_index"}}},
      {"dchain_is_index_allocated", {{0, "dchain_is_index_allocated"}}},
      {"map_allocate", {{0, ""}}},
      {"map_get", {{0, "map_get_success"}, {1, "map_get_failure"}}},
      {"map_put", {{0, "map_put"}}},
      {"map_erase", {{0, "map_erase"}}},
      {"expire_items_single_map", {{0, "expire_items_single_map"}}},
      {"vector_allocate", {{0, ""}}},
      {"vector_borrow", {{0, "vector_borrow"}}},
      {"vector_return", {{0, "vector_return"}}},
      {"lpm_init", {{0, ""}}},
      {"lpm_lookup", {{0, "lpm_lookup"}}},
      {"trace_reset_buffers", {{0, "trace_reset_buffers"}}},
      {"handle_packet_timestamp", {{0, "handle_packet_timestamp"}}},
      {"lb_find_preferred_available_backend",
       {{0, "lb_find_preferred_available_backend"}}},
      {"flood", {{0, "flood"}}},
      {"nf_set_ipv4_checksum", {{0, "checksum"}}},
      {"process_ip_packet", {{0, "process_ip_packet"}}},
      {"bpf_map_lookup_elem",{{0, "bpf_map_lookup_elem"}}},
      {"bpf_map_update_elem", {{0, "bpf_map_update_elem"}}},
  };
  for (auto it : perf_formula_fn_names) {
    for (auto it1 : it.second) {
      supported_pcv_symbols[FN_CALLS].push_back(it1.second);
    }
  }
  supported_pcv_symbols[FN_CALLS].push_back("constant");

  std::cerr << "Loading Performance Contracts." << std::endl;
}
/* **************************************** */
std::map<std::string, std::string> contract_get_user_variables() {
  return user_variables;
}
/* **************************************** */
std::set<std::string> contract_get_contracts() {
  return std::set<std::string>(fn_names.begin(), fn_names.end());
}
/* **************************************** */
std::set<std::string> contract_get_metrics() {
  return std::set<std::string>(supported_metrics.begin(),
                               supported_metrics.end());
}
/* **************************************** */
bool contract_has_contract(std::string function_name) {
  if (std::find(fn_names.begin(), fn_names.end(), function_name) !=
      fn_names.end()) {
    return true;
  }
  return false;
}
/* **************************************** */
std::map<std::string, std::set<std::string>>
contract_get_optimization_variables() {
  return optimization_variables;
}
/* **************************************** */
std::set<std::string> contract_get_contract_symbols(std::string function_name) {
  return std::set<std::string>(fn_variables[function_name].begin(),
                               fn_variables[function_name].end());
}
/* **************************************** */
std::set<std::string> contract_get_symbols() {
  return {

      /* Map symbols */
      "array map_capacity[4] : w32 -> w8 = symbolic",
      "array current_map_capacity[4] : w32 -> w8 = symbolic",
      "array initial_map_capacity[4] : w32 -> w8 = symbolic",
      "array map_occupancy[4] : w32 -> w8 = symbolic",
      "array current_map_occupancy[4] : w32 -> w8 = symbolic",
      "array initial_map_occupancy[4] : w32 -> w8 = symbolic",
      "array Num_bucket_traversals[4] : w32 -> w8 = symbolic",
      "array current_Num_bucket_traversals[4] : w32 -> w8 = symbolic",
      "array initial_Num_bucket_traversals[4] : w32 -> w8 = symbolic",
      "array Num_hash_collisions[4] : w32 -> w8 = symbolic",
      "array current_Num_hash_collisions[4] : w32 -> w8 = symbolic",
      "array initial_Num_hash_collisions[4] : w32 -> w8 = symbolic",
      "array map_has_this_key[4] : w32 -> w8 = symbolic",
      "array current_map_has_this_key[4] : w32 -> w8 = symbolic",
      "array initial_map_has_this_key[4] : w32 -> w8 = symbolic",
      "array map_key_cached[4] : w32 -> w8 = symbolic",
      "array current_map_key_cached[4] : w32 -> w8 = symbolic",
      "array initial_map_key_cached[4] : w32 -> w8 = symbolic",
      "array map_hash[8] : w32 -> w8 = symbolic",
      "array current_map_hash[8] : w32 -> w8 = symbolic",
      "array initial_map_hash[8] : w32 -> w8 = symbolic",
      "array map_key_eq[8] : w32 -> w8 = symbolic",
      "array current_map_key_eq[8] : w32 -> w8 = symbolic",
      "array initial_map_key_eq[8] : w32 -> w8 = symbolic",
      "array map_values[4096] : w32 -> w8 = symbolic",
      "array current_map_values[4096] : w32 -> w8 = symbolic",
      "array initial_map_values[4096] : w32 -> w8 = symbolic",

      /* Double Chain symbols */
      "array dchain_out_of_space[4] : w32 -> w8 = symbolic",
      "array current_dchain_out_of_space[4] : w32 -> w8 = symbolic",
      "array initial_dchain_out_of_space[4] : w32 -> w8 = symbolic",

      /* Expirator symbols */
      "array expired_flows[4] : w32 -> w8 = symbolic",
      "array current_expired_flows[4] : w32 -> w8 = symbolic",
      "array initial_expired_flows[4] : w32 -> w8 = symbolic",

      /* CHT symbols */
      "array backend_capacity[4] : w32 -> w8 = symbolic",
      "array current_backend_capacity[4] : w32 -> w8 = symbolic",
      "array initial_backend_capacity[4] : w32 -> w8 = symbolic",
      "array available_backends[4] : w32 -> w8 = symbolic",
      "array current_available_backends[4] : w32 -> w8 = symbolic",
      "array initial_available_backends[4] : w32 -> w8 = symbolic",

      /* Vector symbols */
      "array borrowed_cell[6] : w32 -> w8 = symbolic", // Legacy from Vigor,
                                                       // not
                                                       // required by Bolt
      "array current_borrowed_cell[6] : w32 -> w8 = symbolic",
      "array initial_borrowed_cell[6] : w32 -> w8 = symbolic",

      /* Natasha symbols */
      "array max_lpm_depth[4] : w32 -> w8 = symbolic",
      "array current_max_lpm_depth[4] : w32 -> w8 = symbolic",
      "array initial_max_lpm_depth[4] : w32 -> w8 = symbolic",
      "array lpm_stages[4] : w32 -> w8 = symbolic",
      "array current_lpm_stages[4] : w32 -> w8 = symbolic",
      "array initial_lpm_stages[4] : w32 -> w8 = symbolic",
      "array rewrite_src_ip[4] : w32 -> w8 = symbolic",
      "array current_rewrite_src_ip[4] : w32 -> w8 = symbolic",
      "array initial_rewrite_src_ip[4] : w32 -> w8 = symbolic",
      "array rewrite_dst_ip[4] : w32 -> w8 = symbolic",
      "array current_rewrite_dst_ip[4] : w32 -> w8 = symbolic",
      "array initial_rewrite_dst_ip[4] : w32 -> w8 = symbolic",
      "array matching_rule_found[4] : w32 -> w8 = symbolic",
      "array current_matching_rule_found[4] : w32 -> w8 = symbolic",
      "array initial_matching_rule_found[4] : w32 -> w8 = symbolic",

      /* BPF Map symbols */
      "array bpf_map_type[4] : w32 -> w8 = symbolic",
      "array current_bpf_map_type[4] : w32 -> w8 = symbolic",
      "array initial_bpf_map_type[4] : w32 -> w8 = symbolic",

      /* Other symbols */
      "array incoming_package[4] : w32 -> w8 = symbolic",
      "array current_incoming_package[4] : w32 -> w8 = symbolic",
      "array initial_incoming_package[4] : w32 -> w8 = symbolic",
      "array user_buf_addr[4] : w32 -> w8 = symbolic",
      "array current_user_buf_addr[4] : w32 -> w8 = symbolic",
      "array initial_user_buf_addr[4] : w32 -> w8 = symbolic",
      "array mbuf[4] : w32 -> w8 = symbolic",
      "array current_mbuf[4] : w32 -> w8 = symbolic",
      "array initial_mbuf[4] : w32 -> w8 = symbolic",
      "array multi_stage_lookup[4] : w32 -> w8 = symbolic",
      "array current_multi_stage_lookup[4] : w32 -> w8 = symbolic",
      "array initial_multi_stage_lookup[4] : w32 -> w8 = symbolic",
      "array recent_flow[4] : w32 -> w8 = symbolic",
      "array current_recent_flow[4] : w32 -> w8 = symbolic",
      "array initial_recent_flow[4] : w32 -> w8 = symbolic",
  };
}
/* **************************************** */
int contract_get_symbol_size(std::string symbol_name) {
  return std::map<std::string, int>({

      /* Map symbols */
      {"map_capacity", 4},
      {"map_occupancy", 4},
      {"Num_bucket_traversals", 4},
      {"Num_hash_collisions", 4},
      {"map_has_this_key", 4},
      {"map_key_cached", 4},
      {"map_hash", 8},
      {"map_key_eq", 8},

      /* Double Chain symbols */
      {"dchain_out_of_space", 4},

      /* Expirator symbols */
      {"expired_flows", 4},

      /* CHT symbols */
      {"backend_capacity", 4},
      {"available_backends", 4},

      /* Vector symbols */
      {"borrowed_cell", 6}, // Legacy from Vigor,
                            // not
                            // required by Bolt
      {"current_borrowed_cell", 6},

      /* Natasha symbols */
      {"max_lpm_depth", 4},
      {"lpm_stages", 4},
      {"rewrite_src_ip", 4},
      {"rewrite_dst_ip", 4},
      {"matching_rule_found", 4},

      /* BPF map symbols */
      {"bpf_map_type",4},

      /* Other symbols */
      {"incoming_package", 4},
      {"user_buf_addr", 4},
      {"mbuf", 4},
      {"multi_stage_lookup", 4},
      {"recent_flow", 4},
  })[symbol_name];
}
/* **************************************** */
int contract_num_sub_contracts(std::string function_name) {
  return constraints[function_name].size();
}
/* **************************************** */
std::string contract_get_subcontract_constraints(std::string function_name,
                                                 int sub_contract_idx) {
  return constraints[function_name][sub_contract_idx];
}
/* **************************************** */
long contract_get_sub_contract_performance(
    std::string function_name, int sub_contract_idx, std::string metric,
    std::map<std::string, long> variables) {
  if (!(check_metric(metric)))
    return -1;

  bool res = get_relevant_vars(function_name, variables);
  if (!res)
    return -1;

  perf_calc_fn_ptr fn_ptr;
  fn_ptr = perf_fn_ptrs[function_name][sub_contract_idx];
  assert(fn_ptr);
  long perf = fn_ptr(metric, relevant_vals);
  if (metric == "memory instructions" && perf > 0) {
    perf = perf - 1;
  }
  return perf;
}

/* **************************************** */
std::map<std::string, std::set<int>>
contract_get_concrete_state(std::string function_name, int sub_contract_idx,
                            std::map<std::string, long> variables) {

  bool res = get_relevant_vars(function_name, variables);
  assert(res && "Failed to provide necessary variables for contract");

  cstate_fn_ptr fn_ptr;
  fn_ptr = cstate_fn_ptrs[function_name][sub_contract_idx];
  assert(fn_ptr && "Function pointer not found for cstate call\n");

  std::map<std::string, std::set<int>> cstate = fn_ptr(relevant_vals);
  return cstate;
}

/* **************************************** */
perf_formula contract_add_perf_formula(perf_formula accumulator,
                                       perf_formula addend,
                                       PCVAbstraction PCVAbs) {
  return add_perf_formula(accumulator, addend, PCVAbs);
}

/* **************************************** */
perf_formula contract_get_perf_formula(std::string function_name,
                                       int sub_contract_idx, std::string metric,
                                       std::map<std::string, long> variables,
                                       PCVAbstraction PCVAbs) {

  assert(check_metric(metric) && "Invalid metric");
  bool res = get_relevant_vars(function_name, variables);
  assert(res && "Failed to provide necessary variables for contract");

  perf_formula_ptr fn_ptr =
      perf_formula_fn_ptrs[function_name][sub_contract_idx];
  assert(fn_ptr && "Function pointer not found for formula call\n");

  perf_formula formula = fn_ptr(metric, relevant_vals, PCVAbs);
  if (formula.count("constant") == 0)
    formula["constant"] = 0;
  assert(check_formula(formula, PCVAbs));
  return formula;
}
/* **************************************** */

std::string contract_display_perf_formula(perf_formula formula,
                                          PCVAbstraction PCVAbs) {
  return display_perf_formula(formula, PCVAbs);
}

int main() {}
