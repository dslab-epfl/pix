#include "klee/klee.h"
#include <stdlib.h>
#include <string.h>

/* This is the same list of PCVs and OVs as in the contract file. This is
 * unfortunate duplication and should be fixed */

int traced_variable_type(char *variable, char **type) {
  *type = malloc(4 * sizeof(char));
  if (!strcmp(variable, "Num_bucket_traversals") ||
      !strcmp(variable, "Num_hash_collisions") ||
      !strcmp(variable, "map_occupancy") || !strcmp(variable, "map_capacity") ||
      !strcmp(variable, "dmap_capacity") ||
      !strcmp(variable, "dmap_occupancy") ||
      !strcmp(variable, "expired_flows") ||
      !strcmp(variable, "backend_capacity") ||
      !strcmp(variable, "lpm_stages") || !strcmp(variable, "max_lpm_depth") ||
      !strcmp(variable, "available_backends")) {
    strcpy(*type, "PCV");
    return 1;
  } else if (!strncmp(
                 variable, "map_has_this_key_occurence",
                 strlen("map_has_this_key_occurence")) || /* Has n versions */
             !strncmp(
                 variable, "map_key_cached_occurence",
                 strlen("map_key_cached_occurence")) || /* Has n versions */
             !strcmp(variable, "dmap_has_this_key") ||
             !strcmp(variable, "multi_stage_lookup") ||
             !strcmp(variable, "recent_flow") ||
             !strcmp(variable, "dchain_out_of_space") ||
             !strcmp(variable, "rewrite_src_ip") ||
             !strcmp(variable, "rewrite_dst_ip") ||
             !strcmp(variable, "matching_rule_found") ||
             !strcmp(variable, "dchain_out_of_space") ||
             !strcmp(variable, "map_hash") || !strcmp(variable, "map_key_eq")) {
    strcpy(*type, "SV");
    return 1;
  } else if (!strcmp(variable, "map") || !strcmp(variable, "dmap") ||
             !strcmp(variable, "dchain") || !strcmp(variable, "cht")) {
    strcpy(*type, "DS");
    return 1;
  }
  return 0;
}

/* Including some path determination functions in here */
#define DEF_DS_PATH(num)                                                       \
  __attribute__((noinline)) void ds_path_##num() {}
DEF_DS_PATH(1);
DEF_DS_PATH(2);
DEF_DS_PATH(3);
#undef DEF_DS_PATH
