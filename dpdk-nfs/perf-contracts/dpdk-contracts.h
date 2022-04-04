/* Contract for DPDK Hack for freeing buffers */

#include "contract-params.h"

/* Perf contracts */

long trace_reset_buffers_contract_0(std::string metric,
                                    std::vector<long> values);

long flood_contract_0(std::string metric, std::vector<long> values);

long checksum_contract_0(std::string metric, std::vector<long>values);

/* Cstate contracts */

std::map<std::string, std::set<int>>
trace_reset_buffers_cstate_contract_0(std::vector<long> values);

std::map<std::string, std::set<int>>
flood_cstate_contract_0(std::vector<long> values);

std::map<std::string, std::set<int>>
checksum_cstate_contract_0(std::vector<long> values);

/* Perf Formula contracts */

perf_formula trace_reset_buffers_formula_contract_0(std::string metric,
                                                    std::vector<long> values,
                                                    PCVAbstraction PCVAbs);

perf_formula flood_formula_contract_0(std::string metric,
                                      std::vector<long> values,
                                      PCVAbstraction PCVAbs);

perf_formula checksum_formula_contract_0(std::string metric,
                                      std::vector<long> values,
                                      PCVAbstraction PCVAbs);