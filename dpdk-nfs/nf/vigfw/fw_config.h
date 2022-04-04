#pragma once

#include <stdint.h>

#include "lib/nf_util.h" /* For enabling traffic classes */
#include "lib/ethtool.h"
#include <rte_ether.h>
#include <rte_config.h>

#define CONFIG_FNAME_LEN 512

struct fw_config {
  // LAN (i.e. internal) device
  uint16_t lan_device;

  // WAN device, i.e. external
  uint16_t wan_device;

  // Size of the dynamic filtering table
  uint32_t capacity;

  // Expiration time of flows in nanoseconds
  uint32_t expiration_time;

   // MAC addresses of devices
  struct ether_addr device_macs[RTE_MAX_ETHPORTS];

  // MAC addresses of the endpoints the devices are linked to
  struct ether_addr endpoint_macs[RTE_MAX_ETHPORTS];
};

void fw_config_init(struct fw_config *config, int argc, char **argv);

void fw_config_cmdline_print_usage(void);

void fw_print_config(struct fw_config *config);

enum TrafficClass {
  UNDEFINED,
  PACKET_RECEIVED,
  EXTERNAL,
  INTERNAL,
  DROPPED, 
  INTERNAL_NEW,
};

extern enum TrafficClass NF_TRAFFIC_CLASS;
