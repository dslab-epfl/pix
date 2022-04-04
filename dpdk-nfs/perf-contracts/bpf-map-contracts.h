#include "contract-params.h"

/* Perf contracts */
long bpf_map_lookup_elem_contract_0(std::string metric, std::vector<long> values);

long bpf_map_update_elem_contract_0(std::string metric, std::vector<long> values);


/* Cstate contracts */
std::map<std::string, std::set<int>>
bpf_map_lookup_elem_cstate_contract_0(std::vector<long> values);

std::map<std::string, std::set<int>>
bpf_map_update_elem_cstate_contract_0(std::vector<long> values);

/* Perf Formula contracts */
perf_formula bpf_map_lookup_elem_formula_contract_0(
    std::string metric, std::vector<long> values, PCVAbstraction PCVAbs);

perf_formula bpf_map_update_elem_formula_contract_0(
    std::string metric, std::vector<long> values, PCVAbstraction PCVAbs);