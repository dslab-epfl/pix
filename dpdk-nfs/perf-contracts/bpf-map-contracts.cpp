#include "bpf-map-contracts.h"
#include "assert.h"

/* Perf contracts */

long bpf_map_lookup_elem_contract_0(std::string metric, std::vector<long> values) {
#ifdef PROVING_CONTENTION
  long bpf_map_type = values[0];
  if(!(bpf_map_type == 1 || bpf_map_type == 2 || bpf_map_type == 5 ||
    bpf_map_type == 6 || bpf_map_type == 9 || bpf_map_type == 12 || bpf_map_type == 13|| 
    bpf_map_type == 14 || bpf_map_type == 16)){
      // This should be the list of all maps that use RCU locks, and will not block while reading
      assert(0 && "Cannot disprove contention");
  }
  return bpf_map_type; // Dummy
#endif
  return 0;
}

long bpf_map_update_elem_contract_0(std::string metric, std::vector<long> values) {

#ifdef PROVING_CONTENTION
  long bpf_map_type = values[0];
  if(!(bpf_map_type == 5 || bpf_map_type == 6 || bpf_map_type == 10)){
      // All per-cpu maps
      assert(0 && "Cannot disprove contention");
  }

  return bpf_map_type; // Dummy
#endif
  return 0;
}

/* Cstate contracts */
std::map<std::string, std::set<int>> bpf_map_lookup_elem_cstate_contract_0(std::vector<long> values) {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>> bpf_map_update_elem_cstate_contract_0(std::vector<long> values) {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

/* Perf Formula contracts */
perf_formula bpf_map_lookup_elem_formula_contract_0(
    std::string metric, std::vector<long> values, PCVAbstraction PCVAbs) {

  perf_formula formula;
  if (PCVAbs == LOOP_CTRS){
    formula["constant"] = bpf_map_lookup_elem_contract_0(metric, values);
  }
  else
    formula["bpf_map_lookup_elem"] = 1;
  return formula;
}

perf_formula bpf_map_update_elem_formula_contract_0(
    std::string metric, std::vector<long> values, PCVAbstraction PCVAbs) {

  perf_formula formula;
  if (PCVAbs == LOOP_CTRS){
    formula["constant"] = bpf_map_update_elem_contract_0(metric, values);
  }
  else
    formula["bpf_map_update_elem"] = 1;
  return formula;
}