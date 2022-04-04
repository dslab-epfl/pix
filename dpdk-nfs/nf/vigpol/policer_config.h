#pragma once

#include <stdint.h>

#include "lib/nf_util.h" /* For enabling traffic classes */
#include "lib/ethtool.h"
#include <rte_ether.h>
#include <rte_config.h>

#define CONFIG_FNAME_LEN 512

struct policer_config {
  // LAN (i.e. internal) device
  uint16_t lan_device;

  // WAN device, i.e. external
  uint16_t wan_device;

  // Policer rate in B/s
  uint64_t rate;

  // Policer burst size in B
  uint64_t burst;

  // Size of the dynamic filtering table
  uint32_t dyn_capacity;

   // MAC addresses of devices
  struct ether_addr device_macs[RTE_MAX_ETHPORTS];

  // MAC addresses of the endpoints the devices are linked to
  struct ether_addr endpoint_macs[RTE_MAX_ETHPORTS];
};

void policer_config_init(struct policer_config *config, int argc, char **argv);

void policer_config_cmdline_print_usage(void);

void policer_print_config(struct policer_config *config);

enum TrafficClass {
  UNDEFINED,
  PACKET_RECEIVED,
  EXTERNAL,
  INTERNAL,
  PERMITTED, 
  RATE_LIMITED,
};

extern enum TrafficClass NF_TRAFFIC_CLASS;
