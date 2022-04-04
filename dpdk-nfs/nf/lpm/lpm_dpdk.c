#include "lpm.h"

#include <stdio.h>
#include <arpa/inet.h>
#include <rte_lpm.h>
#include <rte_lcore.h>

static const unsigned LPM_MAX_RULES = 1e6;
static const unsigned LPM_NUMBER_TBL8S = 1 << 8;


void lpm_init(const char fname[], void **lpm_out) {
  struct rte_lpm_config config_lpm;
  config_lpm.max_rules = LPM_MAX_RULES;
  config_lpm.number_tbl8s = LPM_NUMBER_TBL8S;
  config_lpm.flags = 0;
  *lpm_out = rte_lpm_create("route_table", rte_socket_id(), &config_lpm);
  if (*lpm_out == NULL) {
    rte_exit(EXIT_FAILURE, "Cannot allocate the LPM table on socket %d",
             rte_socket_id());
  }

  FILE *pfx2as_file = fopen(fname, "r");
  if (pfx2as_file == NULL) {
    rte_exit(EXIT_FAILURE, "Error opening pfx2as file: %s.\n", fname);
  }

  for (unsigned long count = 0; count < LPM_MAX_RULES; ++count) {
    char ip_str[INET_ADDRSTRLEN];
    uint32_t ip;
    uint8_t depth;
    int port;

    int result = fscanf(pfx2as_file, "%s", ip_str);
    if (result == EOF) {
      break;
    }

    if (result != 1) {
      rte_exit(EXIT_FAILURE, "Error in ipaddr in pfx2as file %s:%lu\n", fname,
               count);
    }

    result = fscanf(pfx2as_file, "%hh" PRIu8, &depth);
    if (result != 1) {
      rte_exit(EXIT_FAILURE, "Error in prefix detpth in pfx2as file %s:%lu\n",
               fname, count);
    }

    result = fscanf(pfx2as_file, "%d_", &port);
    if (result == 1) {
      while (getc(pfx2as_file) != '\n')
        continue;
    }
    inet_pton(AF_INET, ip_str, &ip);
    result = rte_lpm_add(*lpm_out, rte_be_to_cpu_32(ip), depth, port);
    if (result < 0) {
      rte_exit(EXIT_FAILURE, "Cannot add entry %lu to the LPM table.", count);
    }
  }
  fclose(pfx2as_file);
}

uint32_t lpm_lookup(void *lpm, uint32_t addr) {
  uint32_t next_hop;
  int result = rte_lpm_lookup(lpm, addr, &next_hop);
  return result == 0 ? next_hop : -1;
}
