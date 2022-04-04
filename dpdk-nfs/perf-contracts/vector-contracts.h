#include "contract-params.h"

/* Perf contracts */

long vector_allocate_contract_0(std::string metric, std::vector<long> values);

long vector_borrow_contract_0(std::string metric, std::vector<long> values);

long vector_return_contract_0(std::string metric, std::vector<long> values);

/* Cstate contracts */

std::map<std::string, std::set<int>>
vector_allocate_cstate_contract_0(std::vector<long> values);

std::map<std::string, std::set<int>>
vector_borrow_cstate_contract_0(std::vector<long> values);

std::map<std::string, std::set<int>>
vector_return_cstate_contract_0(std::vector<long> values);

/* Perf Formula contracts */

perf_formula vector_allocate_formula_contract_0(std::string metric,
                                                std::vector<long> values,
                                                PCVAbstraction PCVAbs);

perf_formula vector_borrow_formula_contract_0(std::string metric,
                                              std::vector<long> values, PCVAbstraction PCVAbs);

perf_formula vector_return_formula_contract_0(std::string metric,
                                              std::vector<long> values, PCVAbstraction PCVAbs);
