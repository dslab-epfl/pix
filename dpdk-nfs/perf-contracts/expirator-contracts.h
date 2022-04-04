#include "contract-params.h"
#include "dchain-contracts.h"
#include "map-contracts.h"
#include "vector-contracts.h"

/* Perf contracts */

long expire_items_contract_0(std::string metric, std::vector<long> values);

long expire_items_single_map_contract_0(std::string metric,
                                        std::vector<long> values);

/* Cstate contracts */

std::map<std::string, std::set<int>>
expire_items_cstate_contract_0(std::vector<long> values);

std::map<std::string, std::set<int>>
expire_items_single_map_cstate_contract_0(std::vector<long> values);

/* Perf Formula contracts */

perf_formula expire_items_formula_contract_0(std::string metric,
                                             std::vector<long> values,
                                             PCVAbstraction PCVAbs);

perf_formula expire_items_single_map_formula_contract_0(
    std::string metric, std::vector<long> values, PCVAbstraction PCVAbs);