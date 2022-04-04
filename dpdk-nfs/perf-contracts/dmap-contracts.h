#include "contract-params.h"
#include "map-impl-contracts.h"

/* Perf contracts */

long dmap_allocate_contract_0(std::string metric, std::vector<long> values);

long dmap_get_a_contract_0(std::string metric, std::vector<long> values);

long dmap_get_a_contract_1(std::string metric, std::vector<long> values);

long dmap_get_a_contract_2(std::string metric, std::vector<long> values);

long dmap_get_b_contract_0(std::string metric, std::vector<long> values);

long dmap_get_b_contract_1(std::string metric, std::vector<long> values);

long dmap_get_b_contract_2(std::string metric, std::vector<long> values);

long dmap_get_value_contract_0(std::string metric, std::vector<long> values);

long dmap_get_value_contract_1(std::string metric, std::vector<long> values);

long dmap_put_contract_0(std::string metric, std::vector<long> values);

long dmap_put_contract_1(std::string metric, std::vector<long> values);

long dmap_erase_contract_0(std::string metric, std::vector<long> values);

long dmap_erase_contract_1(std::string metric, std::vector<long> values);

/* Cstate contracts */

std::map<std::string, std::set<int>>
dmap_allocate_cstate_contract_0(std::vector<long> values);

std::map<std::string, std::set<int>>
dmap_get_a_cstate_contract_0(std::vector<long> values);

std::map<std::string, std::set<int>>
dmap_get_a_cstate_contract_1(std::vector<long> values);

std::map<std::string, std::set<int>>
dmap_get_b_cstate_contract_0(std::vector<long> values);

std::map<std::string, std::set<int>>
dmap_get_b_cstate_contract_1(std::vector<long> values);

std::map<std::string, std::set<int>>
dmap_get_value_cstate_contract_0(std::vector<long> values);

std::map<std::string, std::set<int>>
dmap_put_cstate_contract_0(std::vector<long> values);

std::map<std::string, std::set<int>>
dmap_erase_cstate_contract_0(std::vector<long> values);

/* Perf Formula contracts */

perf_formula dmap_allocate_formula_contract_0(std::string metric,
                                              std::vector<long> values,
                                              PCVAbstraction PCVAbs);

perf_formula dmap_get_formula_contract_0(std::string metric,
                                         std::vector<long> values,
                                         PCVAbstraction PCVAbs);

perf_formula dmap_get_formula_contract_1(std::string metric,
                                         std::vector<long> values,
                                         PCVAbstraction PCVAbs);

perf_formula dmap_get_value_formula_contract_0(std::string metric,
                                               std::vector<long> values,
                                               PCVAbstraction PCVAbs);

perf_formula dmap_put_formula_contract_0(std::string metric,
                                         std::vector<long> values,
                                         PCVAbstraction PCVAbs);

perf_formula dmap_erase_formula_contract_0(std::string metric,
                                           std::vector<long> values,
                                           PCVAbstraction PCVAbs);