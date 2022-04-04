#pragma once

/* Performance contracts for Helper Functions */

#include "contract-params.h"

/* Perf contracts */

long ether_addr_hash_contract(std::string metric);

long ether_addr_eq_contract(std::string metric, long success);

long flow_id_eq_contract(std::string metric, long success);

long ext_key_eq_contract(std::string metric, long success);

long flow_id_hash_contract(std::string metric);

long ext_key_hash_contract(std::string metric);

long flow_cpy_contract(std::string metric, long recent);

long flow_destroy_contract(std::string metric);

long flow_extract_keys_contract(std::string metric);

long flow_pack_keys_contract(std::string metric);

long lb_flow_hash_contract(std::string metric);

long lb_ip_hash_contract(std::string metric);

long lb_flow_equality_contract(std::string metric, long success);

long lb_ip_equality_contract(std::string metric, long success);

long policer_flow_hash_contract(std::string metric);

long policer_flow_eq_contract(std::string metric, long success);

/* Cstate contracts */

std::map<std::string, std::set<int>> ether_addr_hash_cstate_contract();

std::map<std::string, std::set<int>> ether_addr_eq_cstate_contract();

std::map<std::string, std::set<int>> flow_id_hash_cstate_contract();

std::map<std::string, std::set<int>> ext_key_hash_cstate_contract();

std::map<std::string, std::set<int>> flow_id_eq_cstate_contract();

std::map<std::string, std::set<int>> ext_key_eq_cstate_contract();

std::map<std::string, std::set<int>> flow_cpy_cstate_contract();

std::map<std::string, std::set<int>> flow_destroy_cstate_contract();

std::map<std::string, std::set<int>> flow_extract_keys_cstate_contract();

std::map<std::string, std::set<int>> flow_pack_keys_cstate_contract();

std::map<std::string, std::set<int>> lb_flow_hash_cstate_contract();

std::map<std::string, std::set<int>> lb_ip_hash_cstate_contract();

std::map<std::string, std::set<int>> lb_flow_equality_cstate_contract();

std::map<std::string, std::set<int>> lb_ip_equality_cstate_contract();

std::map<std::string, std::set<int>> policer_flow_hash_cstate_contract();

std::map<std::string, std::set<int>> policer_flow_eq_cstate_contract();

/* Perf formula contracts */

perf_formula ether_addr_hash_formula_contract(std::string metric,
                                              PCVAbstraction PCVAbs);

perf_formula ether_addr_eq_formula_contract(std::string metric, long success,
                                            PCVAbstraction PCVAbs);

perf_formula flow_id_eq_formula_contract(std::string metric, long success,
                                         PCVAbstraction PCVAbs);

perf_formula ext_key_eq_formula_contract(std::string metric, long success,
                                         PCVAbstraction PCVAbs);

perf_formula flow_id_hash_formula_contract(std::string metric,
                                           PCVAbstraction PCVAbs);

perf_formula ext_key_hash_formula_contract(std::string metric,
                                           PCVAbstraction PCVAbs);

perf_formula flow_cpy_formula_contract(std::string metric, long recent,
                                       PCVAbstraction PCVAbs);

perf_formula flow_destroy_formula_contract(std::string metric,
                                           PCVAbstraction PCVAbs);

perf_formula flow_extract_keys_formula_contract(std::string metric,
                                                PCVAbstraction PCVAbs);

perf_formula flow_pack_keys_formula_contract(std::string metric,
                                             PCVAbstraction PCVAbs);

perf_formula lb_flow_hash_formula_contract(std::string metric,
                                           PCVAbstraction PCVAbs);

perf_formula lb_ip_hash_formula_contract(std::string metric,
                                         PCVAbstraction PCVAbs);

perf_formula lb_flow_equality_formula_contract(std::string metric, long success,
                                               PCVAbstraction PCVAbs);

perf_formula lb_ip_equality_formula_contract(std::string metric, long success,
                                             PCVAbstraction PCVAbs);

perf_formula policer_flow_hash_formula_contract(std::string metric,
                                             PCVAbstraction PCVAbs);
perf_formula policer_flow_eq_formula_contract(std::string metric, long success,
                                             PCVAbstraction PCVAbs);

