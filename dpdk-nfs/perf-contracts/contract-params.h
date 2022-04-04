#pragma once
/* Hardware parameters used by all contracts */

/* Hardware parameters */

#define DRAM_LATENCY 200
#define L1_LATENCY 2

/* ABI */

#ifndef CONTRACT_PARAMS_H
#define CONTRACT_PARAMS_H

#define _GLIBCXX_USE_CXX11_ABI 0

/* Common includes */
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <vector>

/* Definition for cstate stitching function, required by all contracts */

std::map<std::string, std::set<int>>
add_cstate_dependency(std::map<std::string, std::set<int>> caller,
                      std::map<std::string, int> caller_state,
                      std::map<std::string, std::set<int>> callee);

/* Common type definition for all perf contracts */
typedef long (*perf_calc_fn_ptr)(std::string, std::vector<long>);

/* Common type definition for all cstate contracts */
typedef std::map<std::string, std::set<int>> (*cstate_fn_ptr)(
    std::vector<long>);

/* Common type definition for all perf formula */
typedef std::map<std::string, long> perf_formula;

#ifndef PCV_ABSTRACTION
/* Defining PCV Abstraction Levels */
typedef enum {
  LOOP_CTRS, // Collisions, Traversals etc.
  FN_CALLS,  // LibVig Function calls
} PCVAbstraction;
#endif

/* Common type definition for all perf formula contracts */
typedef perf_formula (*perf_formula_ptr)(std::string, std::vector<long>,
                                         PCVAbstraction PCVAbs);

/* Definition for perf formula addition function, required by all contracts */
perf_formula add_perf_formula(perf_formula ip1, perf_formula ip2,
                              PCVAbstraction PCVAbs);

/* Definition for perf formula multiplication function, required by some
 * contracts */
perf_formula multiply_perf_formula(perf_formula ip1, perf_formula ip2,
                                   PCVAbstraction PCVAbs);

/* Definition for perf formula addition display, required by all contracts */
std::string display_perf_formula(perf_formula formula, PCVAbstraction PCVAbs);

/* Sanity check for perf formula required by all contracts */
bool check_formula(perf_formula formula, PCVAbstraction PCVAbs);

#endif // CONTRACT_PARAMS_H