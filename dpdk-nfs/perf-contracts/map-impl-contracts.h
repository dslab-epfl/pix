/* Performance contracts for Map-Impl functions */

#include "contract-params.h"
#include "helper-contracts.h"

/* Perf contracts */

typedef long (*map_key_eq_ptr)(std::string, long success);
typedef std::map<std::string, std::set<int>> (*helper_cstate_fn_ptr)();
typedef perf_formula (*map_key_eq_formula_ptr)(std::string, long success,
                                               PCVAbstraction PCV);

long map_impl_init_contract(std::string metric, long success, long capacity);

long map_impl_put_contract(std::string metric, long recent,
                           long num_traversals);

long map_impl_get_contract(std::string metric, long success, long recent,
                           long map_key_eq, long num_traversals,
                           long num_collisions);

long map_impl_erase_contract(std::string metric, long recent, long map_key_eq,
                             long num_traversals, long num_collisions);

/* Cstate contracts */

std::map<std::string, std::set<int>>
map_impl_init_cstate_contract(long success, long capacity);

std::map<std::string, std::set<int>>
map_impl_put_cstate_contract(long num_traversals);

std::map<std::string, std::set<int>>
map_impl_get_cstate_contract(long success, long map_key_eq, long num_traversals,
                             long num_collisions);

std::map<std::string, std::set<int>>
map_impl_erase_cstate_contract(long map_key_eq, long num_traversals,
                               long num_collisions);

/* Perf Formula contracts */

perf_formula map_impl_init_formula_contract(std::string metric, long success,
                                            long capacity,
                                            PCVAbstraction PCVAbs);

perf_formula map_impl_put_formula_contract(std::string metric, long recent,
                                           long num_traversals,
                                           PCVAbstraction PCVAbs);

perf_formula map_impl_get_formula_contract(std::string metric, long success,
                                           long recent, long map_key_eq,
                                           long num_traversals,
                                           long num_collisions,
                                           PCVAbstraction PCVAbs);

perf_formula map_impl_erase_formula_contract(std::string metric, long recent,
                                             long map_key_eq,
                                             long num_traversals,
                                             long num_collisions,
                                             PCVAbstraction PCVAbs);