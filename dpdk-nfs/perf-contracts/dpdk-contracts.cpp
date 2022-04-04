/* Contract for DPDK Hack for freeing buffers */

#include "dpdk-contracts.h"

/* Perf contracts */

long trace_reset_buffers_contract_0(std::string metric,
                                    std::vector<long> values) {
  long constant;
  if (metric == "instruction count")
    constant = 93;
  else if (metric == "memory instructions")
    constant = 15;
  else if (metric == "execution cycles")
    constant = 8 * DRAM_LATENCY + 7 * L1_LATENCY + 51;
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}

long flood_contract_0(std::string metric, std::vector<long> values) {
  long constant = 0;
  if (metric == "instruction count") {
    constant = 309;
  } else if (metric == "memory instructions") {
    constant = 177;
  } else if (metric == "execution cycles") {
    constant = 4 * DRAM_LATENCY + 173 * L1_LATENCY + 147;
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}

long checksum_contract_0(std::string metric, std::vector<long> values) {
  long constant = 0;
  if (metric == "instruction count") {
    constant = 118;
  } else if (metric == "memory instructions") {
    constant = 26;
  } else if (metric == "execution cycles") {
    constant = 26 * L1_LATENCY + 147;
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}

/* Cstate contracts */

std::map<std::string, std::set<int>>
trace_reset_buffers_cstate_contract_0(std::vector<long> values) {

  /* TODO */
  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>>
flood_cstate_contract_0(std::vector<long> values) {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

std::map<std::string, std::set<int>>
checksum_cstate_contract_0(std::vector<long> values) {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

/* Perf Formula contracts */

perf_formula trace_reset_buffers_formula_contract_0(std::string metric,
                                                    std::vector<long> values,
                                                    PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = trace_reset_buffers_contract_0(metric, values);
  else if (PCVAbs == FN_CALLS)
    formula["trace_reset_buffers"] = 1;
  return formula;
}

perf_formula flood_formula_contract_0(std::string metric,
                                      std::vector<long> values,
                                      PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = flood_contract_0(metric, values);
  else if (PCVAbs == FN_CALLS)
    formula["flood"] = 1;
  return formula;
}

perf_formula checksum_formula_contract_0(std::string metric,
                                      std::vector<long> values,
                                      PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = checksum_contract_0(metric, values);
  else if (PCVAbs == FN_CALLS)
    formula["flood"] = 1;
  return formula;
}