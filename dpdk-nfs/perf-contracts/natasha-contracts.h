#include "contract-params.h"

/* Perf contracts */

long process_ip_packet_contract_0(std::string metric, std::vector<long> values);

/* Cstate contracts */

std::map<std::string, std::set<int>>
process_ip_packet_cstate_contract_0(std::vector<long> values);

/* Perf Formula contracts */

perf_formula process_ip_packet_formula_contract_0(std::string metric,
                                         std::vector<long> values,
                                         PCVAbstraction PCVAbs);
