#include "lpm-contracts.h"

/* Perf contracts */

long lpm_init_contract_0(std::string metric, std::vector<long> values) {
  return 0;
}
long lpm_lookup_contract_0(std::string metric, std::vector<long> values) {
  long multi_stage_lookup = values[0];
  long constant;
  if (metric == "instruction count") {
    constant = (multi_stage_lookup == 1) ? 19 : 13;
  } else if (metric == "memory instructions") {
    constant = (multi_stage_lookup == 1) ? 4 : 2;
  } else if (metric == "execution cycles") {
    constant = (multi_stage_lookup == 1)
                   ? 3 * DRAM_LATENCY + 1 * L1_LATENCY + 15
                   : 1 * DRAM_LATENCY + 1 * L1_LATENCY + 11;
  } else if (metric == "llvm instruction count") {
    constant = (multi_stage_lookup == 1) ? 24 : 14;
  } else if (metric == "llvm memory instructions") {
    constant = (multi_stage_lookup == 1) ? 3 : 1;
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}

/* Cstate contracts */

std::map<std::string, std::set<int>>
lpm_init_cstate_contract_0(std::vector<long> values) {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}
std::map<std::string, std::set<int>>
lpm_lookup_cstate_contract_0(std::vector<long> values) {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

/* Perf Formula contracts */

perf_formula lpm_init_formula_contract_0(std::string metric,
                                         std::vector<long> values,
                                         PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = 0;
  return formula;
}

perf_formula lpm_lookup_formula_contract_0(std::string metric,
                                           std::vector<long> values,
                                           PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = lpm_lookup_contract_0(metric, values);
  else if (PCVAbs == FN_CALLS)
    formula["lpm_lookup"] = 1;
  return formula;
}