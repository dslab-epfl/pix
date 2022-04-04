#include "natasha-contracts.h"

/* Perf contracts */

long process_ip_packet_contract_0(std::string metric, std::vector<long> values){
  return 1;
}

/* Cstate contracts */

std::map<std::string, std::set<int>>
process_ip_packet_cstate_contract_0(std::vector<long> values){
  std::map<std::string, std::set<int>> cstate;
  return cstate;
}

/* Perf Formula contracts */

perf_formula process_ip_packet_formula_contract_0(std::string metric,
                                         std::vector<long> values,
                                         PCVAbstraction PCVAbs){
  perf_formula formula;
  if (PCVAbs == LOOP_CTRS){
    formula["constant"] = process_ip_packet_contract_0(metric, values);
    formula["s"] = 20;
  }
  else
    formula["process_ip_packet"] = 1;
  return formula;
  }
