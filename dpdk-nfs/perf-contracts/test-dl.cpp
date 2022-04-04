#include "klee/perf-contracts.h"
#include <assert.h>
#include <dlfcn.h>
#include <iostream>
#include <vector>
#include <algorithm>

int main(int argc, char *argv[]) {
  assert(argc == 2);
  const char *plugin_file = argv[1];
  std::cout << "Loading plugin: " << plugin_file << std::endl;

  dlerror();
  const char *err = NULL;
  void *plugin = dlopen(plugin_file, RTLD_NOW);
  if ((err = dlerror())) {
    std::cout << "Error loading plugin: " << err << std::endl;
    exit(-1);
  }
  assert(plugin);

  /* Testing init */
  std::cout << "Testing init" << std::endl;

  decltype(&contract_init) init =
      (decltype(&contract_init))dlsym(plugin, STRINGIFY(contract_init));
  if ((err = dlerror())) {
    std::cout << "Error loading symbol: " << err << std::endl;
    exit(-1);
  }
  assert(init);
  std::cout << "Init successful" << std::endl;

  init();

  /* Testing get metrics */
  std::cout << "Testing get_metrics" << std::endl;

  decltype(&contract_get_metrics) get_metrics =
      (decltype(&contract_get_metrics))dlsym(plugin,
                                             STRINGIFY(contract_get_metrics));
  if ((err = dlerror())) {
    std::cout << "Error loading symbol: " << err << std::endl;
    exit(-1);
  }
  assert(get_metrics);

  std::set<std::string> metrics = get_metrics();
  std::set<std::string> expected_metrics = {
      "instruction count", "memory instructions", "execution cycles", "llvm instruction count", "llvm memory instructions"};
  assert(metrics == expected_metrics);
  std::cout << "get_metrics test successful" << std::endl;

  /* Testing has_contract */
  std::cout << "Testing has_contract" << std::endl;

  decltype(&contract_has_contract) has_contract =
      (decltype(&contract_has_contract))dlsym(plugin,
                                              STRINGIFY(contract_has_contract));
  if ((err = dlerror())) {
    std::cout << "Error loading symbol: " << err << std::endl;
    exit(-1);
  }
  assert(has_contract);

  bool x = has_contract("dchain_is_index_allocated");
  bool y = has_contract("expire_items");
  bool z = has_contract("Hullo");

  assert(x && y && !z);
  std::cout << "has_contract test successful" << std::endl;

  /* Testing get_user_variables */
  std::cout << "Testing get_user_variables" << std::endl;

  std::map<std::string, std::string> user_variables;

  decltype(&contract_get_user_variables) get_user_variables =
      (decltype(&contract_get_user_variables))dlsym(
          plugin, STRINGIFY(contract_get_user_variables));
  if ((err = dlerror())) {
    std::cout << "Error loading symbol: " << err << std::endl;
    exit(-1);
  }
  assert(get_user_variables);

  user_variables = get_user_variables();
  std::vector<std::string> expected_uvs = {
      "Num_bucket_traversals", "Num_hash_collisions", "expired_flows",
      "available_backends"};
  for (auto it : expected_uvs) {
    assert(user_variables.find(it) != user_variables.end());
  }

  std::cout << "get_user_variables test successful" << std::endl;

  /* Testing get_optimization_variables */
  std::cout << "Testing get_optimization_variables" << std::endl;

  std::map<std::string, std::set<std::string>> optimization_variables;

  decltype(&contract_get_optimization_variables) get_optimization_variables =
      (decltype(&contract_get_optimization_variables))dlsym(
          plugin, STRINGIFY(contract_get_optimization_variables));
  if ((err = dlerror())) {
    std::cout << "Error loading symbol: " << err << std::endl;
    exit(-1);
  }
  assert(get_optimization_variables);

  optimization_variables = get_optimization_variables();
  std::vector<std::string> expected_ovs = {"dchain_out_of_space",
                                           "map_has_this_key", "map_hash",
                                           "map_key_eq", "multi_stage_lookup"};
  for (auto it : expected_ovs) {
    assert(optimization_variables.find(it) != optimization_variables.end());
  }
  std::cout << "get_optimization_variables test successful" << std::endl;

  /* Testing num_sub_contracts */

  std::cout << "Testing num_sub_contracts" << std::endl;

  decltype(&contract_num_sub_contracts) num_sub_contracts =
      (decltype(&contract_num_sub_contracts))dlsym(
          plugin, STRINGIFY(contract_num_sub_contracts));
  if ((err = dlerror())) {
    std::cout << "Error loading symbol: " << err << std::endl;
    exit(-1);
  }
  assert(num_sub_contracts);
  assert(num_sub_contracts("expire_items") == 1);
  assert(num_sub_contracts("map_get") == 2);
  std::cout << "num_sub_contracts test successful" << std::endl;

  /* Testing get_subcontract_constraints */

  std::cout << "Testing get_sub_contract_constraints" << std::endl;

  std::string subcontract_constraints;

  decltype(&contract_get_subcontract_constraints) get_subcontract_constraints =
      (decltype(&contract_get_subcontract_constraints))dlsym(
          plugin, STRINGIFY(contract_get_subcontract_constraints));
  if ((err = dlerror())) {
    std::cout << "Error loading symbol: " << err << std::endl;
    exit(-1);
  }
  assert(get_subcontract_constraints);

  assert(get_subcontract_constraints("dmap_allocate", 0) == "true");
  assert(get_subcontract_constraints("map_get", 1) ==
         "(Eq 0 (ReadLSB w32 0 current_map_has_this_key))");
  std::cout << "get_sub_contract_constraints test successful" << std::endl;

  /* Testing get_sub_contract_performance */

  std::cout << "Testing get_sub_contract_performance and get_perf_formula"
            << std::endl;

  decltype(
      &contract_get_sub_contract_performance) get_sub_contract_performance =
      (decltype(&contract_get_sub_contract_performance))dlsym(
          plugin, STRINGIFY(contract_get_sub_contract_performance));
  if ((err = dlerror())) {
    std::cout << "Error loading symbol: " << err << std::endl;
    exit(-1);
  }
  assert(get_sub_contract_performance);
  long perf;
  std::vector<std::string> fn_names;
  std::map<std::string, long> variables;
  std::map<std::string, int> subcontract_num;

  fn_names = {
      "dchain_allocate",
      "dchain_allocate_new_index",
      "dchain_rejuvenate_index",
      "dchain_is_index_allocated",
      "map_allocate",
      "map_get",
      "map_put",
      "map_erase",
      "dmap_allocate",
      "dmap_get_a",
      "dmap_get_b",
      "dmap_put",
      "dmap_erase",
      "dmap_get_value",
      "expire_items",
      "expire_items_single_map",
      "vector_allocate",
      "vector_borrow",
      "vector_return",
  };

  variables = {

      {"map_capacity", {65536}},
      {"dmap_capacity", {65536}},
      {"map_occupancy", {65535}},
      {"dmap_occupancy", {65535}},
      {"map_has_this_key", {0}},
      {"dmap_has_this_key", {1}},
      {"Num_bucket_traversals", {1}},
      {"Num_hash_collisions", {0}},
      {"dchain_out_of_space", {0}},
      {"expired_flows", {0}},
      {"map_hash", {2}},
      {"map_key_eq", {2}},
      {"recent_flow", {0}},
      {"map_key_cached", {1}}};

  subcontract_num = {{"dchain_allocate", {0}},
                     {"dchain_allocate_new_index", {0}},
                     {"dchain_rejuvenate_index", {0}},
                     {"dchain_is_index_allocated", {0}},
                     {"map_allocate", {0}},
                     {"map_get", {1}},
                     {"map_put", {0}},
                     {"map_erase", {0}},
                     {"dmap_allocate", {0}},
                     {"dmap_get_a", {0}},
                     {"dmap_get_b", {0}},
                     {"dmap_put", {0}},
                     {"dmap_erase", {0}},
                     {"dmap_get_value", {0}},
                     {"expire_items", {0}},
                     {"expire_items_single_map", {0}},
                     {"vector_allocate", {0}},
                     {"vector_borrow", {0}},
                     {"vector_return", {0}}};

  std::vector<std::string> llvm_supported_fns = {
	    "dchain_is_index_allocated",
		"dchain_allocate_new_index",
		"dchain_rejuvenate_index",
		"expire_items_single_map",
		"map_get",
		"map_put",
		"map_erase"
  };

  std::vector<std::string> llvm_metrics = {"llvm instruction count", "llvm memory instructions"};


  decltype(&contract_get_perf_formula) get_perf_formula =
      (decltype(&contract_get_perf_formula))dlsym(
          plugin, STRINGIFY(contract_get_perf_formula));
  if ((err = dlerror())) {
    std::cout << "Error loading symbol: " << err << std::endl;
    exit(-1);
  }
  assert(get_perf_formula);

  decltype(&contract_display_perf_formula) display_perf_formula =
      (decltype(&contract_display_perf_formula))dlsym(
          plugin, STRINGIFY(contract_display_perf_formula));
  if ((err = dlerror())) {
    std::cout << "Error loading symbol: " << err << std::endl;
    exit(-1);
  }
  assert(display_perf_formula);


  PCVAbstraction PCVAbs = LOOP_CTRS;

  for(std::vector<std::string>::iterator i = fn_names.begin(); i != fn_names.end(); ++i) {
	  for(std::set<std::string>::iterator j = metrics.begin(); j != metrics.end(); ++j) {
		if(std::find(llvm_metrics.begin(), llvm_metrics.end(), *j)!= llvm_metrics.end()){
			if(std::find(llvm_supported_fns.begin(), llvm_supported_fns.end(), *i) == llvm_supported_fns.end()){
				continue;
			}
		}
      perf =
          get_sub_contract_performance(*i, subcontract_num[*i], *j, variables);
      std::cout << "[Performance value] " << *i << " " << *j << " " << perf
                << std::endl;
      std::cout << "[Performance formula] " << *i << " " << *j << " "
                << display_perf_formula(get_perf_formula(*i,
                                                         subcontract_num[*i],
                                                         *j, variables, PCVAbs),
                                        PCVAbs);

    }
  }

  return 0;
}
