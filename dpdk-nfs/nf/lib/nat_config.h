#pragma once

#include <inttypes.h>

#include "lib/nf_util.h" /* For enabling traffic classes */
#include "lib/ethtool.h"
#include <rte_config.h>
#include <rte_ether.h>

struct nat_config {
  // "Main" LAN (i.e. internal) device, used for dumb forwarding
  uint16_t lan_main_device;

  // WAN device, i.e. external
  uint16_t wan_device;

  // External IP address
  uint32_t external_addr;

  // MAC addresses of devices
  struct ether_addr device_macs[RTE_MAX_ETHPORTS];

  // MAC addresses of the endpoints the devices are linked to
  struct ether_addr endpoint_macs[RTE_MAX_ETHPORTS];

  // Driver identifier for each device, based on (https://docs.huihoo.com/doxygen/linux/kernel/3.7/include_2uapi_2linux_2ethtool_8h_source.html#l00082)
  int driver_type; // Ideally, this should also be an array (one for each device, but for now, we assume all NICs are of the same type because klee has a problem with symbolic indices)

  // External port at which to start allocating flows
  // i.e. ports will be allocated in [start_port, start_port + max_flows]
  uint16_t start_port;

  // Expiration time of flows in seconds
  uint32_t expiration_time;

  // Size of the flow table
  uint32_t max_flows;
};

void nat_config_init(struct nat_config *config, int argc, char **argv);

void nat_config_cmdline_print_usage(void);

void nat_print_config(struct nat_config *config);

/* Traffic Classes for VigNAT. This is to enable different ones for the perf
 * clarity test */

#define ENABLE_TC_INVALID ()
#define ENABLE_TC_INTERNAL_NEW ()
#define ENABLE_TC_INTERNAL_NEW_FULL ()
#define ENABLE_TC_EXTERNAL_NEW ()
#define ENABLE_TC_KNOWN ()
#define ENABLE_TC_FLOWS_EXPIRED ()
#define ENABLE_TC_COLLISIONS ()

enum TrafficClass {
  UNDEFINED,
  PACKET_RECEIVED,
  EXTERNAL,
  INTERNAL,
  INVALID,
  VALID,
  INTERNAL_NEW,
  INTERNAL_NEW_FULL,
  EXTERNAL_NEW,
  EXTERNAL_KNOWN,
  INTERNAL_KNOWN,
  IP_OPT
};

extern enum TrafficClass NF_TRAFFIC_CLASS;
