#include "contract-params.h"

/* Perf contracts */

long handle_packet_timestamp_contract_0(std::string metric,
                                        std::vector<long> values);

/* Cstate contracts */

std::map<std::string, std::set<int>>
handle_packet_timestamp_cstate_contract_0(std::vector<long> values);

/* Perf Formula contracts */

perf_formula handle_packet_timestamp_formula_contract_0(
    std::string metric, std::vector<long> values, PCVAbstraction PCVAbs);