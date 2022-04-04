#include "contract-params.h"
#include "dchain-contracts.h"
#include "helper-contracts.h"
#include "vector-contracts.h"

/* Perf contracts */
long lb_find_preferred_available_backend_contract_0(std::string metric,
                                                    std::vector<long> values);

/* Cstate contracts */
std::map<std::string, std::set<int>>
lb_find_preferred_available_backend_cstate_contract_0(std::vector<long> values);

/* Perf Formula contracts */
perf_formula lb_find_preferred_available_backend_formula_contract_0(
    std::string metric, std::vector<long> values, PCVAbstraction PCVAbs);