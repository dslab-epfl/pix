#include "contract-params.h"
#include "map-impl-contracts.h"

/* Perf contracts */

typedef long (*map_hash_ptr)(std::string);
typedef perf_formula (*map_hash_formula_ptr)(std::string,
                                             PCVAbstraction PCVAbs);

long map_allocate_contract_0(std::string metric, std::vector<long> values);

long map_get_contract_0(std::string metric, std::vector<long> values);

long map_get_contract_1(std::string metric, std::vector<long> values);

long map_put_contract_0(std::string metric, std::vector<long> values);

long map_erase_contract_0(std::string metric, std::vector<long> values);

/* Cstate contracts */

std::map<std::string, std::set<int>>
map_allocate_cstate_contract_0(std::vector<long> values);

std::map<std::string, std::set<int>>
map_get_cstate_contract_0(std::vector<long> values);

std::map<std::string, std::set<int>>
map_get_cstate_contract_1(std::vector<long> values);

std::map<std::string, std::set<int>>
map_put_cstate_contract_0(std::vector<long> values);

std::map<std::string, std::set<int>>
map_erase_cstate_contract_0(std::vector<long> values);

/* Perf Formula contracts */

perf_formula map_allocate_formula_contract_0(std::string metric,
                                             std::vector<long> values,
                                             PCVAbstraction PCVAbs);

perf_formula map_get_formula_contract_0(std::string metric,
                                        std::vector<long> values,
                                        PCVAbstraction PCVAbs);

perf_formula map_get_formula_contract_1(std::string metric,
                                        std::vector<long> values,
                                        PCVAbstraction PCVAbs);

perf_formula map_put_formula_contract_0(std::string metric,
                                        std::vector<long> values,
                                        PCVAbstraction PCVAbs);

perf_formula map_erase_formula_contract_0(std::string metric,
                                          std::vector<long> values,
                                          PCVAbstraction PCVAbs);