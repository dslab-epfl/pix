#include "contract-params.h"

/* Perf contracts */

long dchain_expire_one_index_contract(std::string metric,
                                      std::vector<long> values);

long dchain_is_index_allocated_contract_0(std::string metric,
                                          std::vector<long> values);

long dchain_allocate_contract_0(std::string metric, std::vector<long> values);

long dchain_allocate_new_index_contract_0(std::string metric,
                                          std::vector<long> values);

long dchain_allocate_new_index_contract_1(std::string metric,
                                          std::vector<long> values);

long dchain_rejuvenate_index_contract_0(std::string metric,
                                        std::vector<long> values);

/* Cstate contracts */

std::map<std::string, std::set<int>>
dchain_expire_one_index_cstate_contract(std::vector<long> values);

std::map<std::string, std::set<int>>
dchain_is_index_allocated_cstate_contract_0(std::vector<long> values);

std::map<std::string, std::set<int>>
dchain_allocate_cstate_contract_0(std::vector<long> values);

std::map<std::string, std::set<int>>
dchain_allocate_new_index_cstate_contract_0(std::vector<long> values);

std::map<std::string, std::set<int>>
dchain_allocate_new_index_cstate_contract_1(std::vector<long> values);

std::map<std::string, std::set<int>>
dchain_rejuvenate_index_cstate_contract_0(std::vector<long> values);

/* Perf formula contracts */

perf_formula dchain_expire_one_index_formula_contract(std::string metric,
                                                      std::vector<long> values,
                                                      PCVAbstraction PCVAbs);

perf_formula dchain_is_index_allocated_formula_contract_0(
    std::string metric, std::vector<long> values, PCVAbstraction PCVAbs);

perf_formula dchain_allocate_formula_contract_0(std::string metric,
                                                std::vector<long> values,
                                                PCVAbstraction PCVAbs);

perf_formula dchain_allocate_new_index_formula_contract_0(
    std::string metric, std::vector<long> values, PCVAbstraction PCVAbs);

perf_formula dchain_allocate_new_index_formula_contract_1(
    std::string metric, std::vector<long> values, PCVAbstraction PCVAbs);

perf_formula dchain_rejuvenate_index_formula_contract_0(
    std::string metric, std::vector<long> values, PCVAbstraction PCVAbs);
