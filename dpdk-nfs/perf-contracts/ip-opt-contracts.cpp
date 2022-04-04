#include "ip-opt-contracts.h"

/* Perf contracts */

long handle_packet_timestamp_contract_0(std::string metric,
                                        std::vector<long> values) {
  long constant;
  if (metric == "instruction count") {
    constant = 303;
  } else if (metric == "memory instructions") {
    constant = 66;
  } else if (metric == "execution cycles") {
    constant = 20 * DRAM_LATENCY + 46 * L1_LATENCY + 300;
  }
  else {
    assert( 0 && "Contract does not support this metric");
  }
  return constant;
}

/* Cstate contracts */

std::map<std::string, std::set<int>>
handle_packet_timestamp_cstate_contract_0(std::vector<long> values) {

  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

/* Perf Formula contracts */

perf_formula handle_packet_timestamp_formula_contract_0(
    std::string metric, std::vector<long> values, PCVAbstraction PCVAbs) {
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS)
    formula["constant"] = handle_packet_timestamp_contract_0(metric, values);
  else
    formula["handle_packet_timestamp"] = 1;
  return formula;
}