#include "policer_config.h"

#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// DPDK needs these but doesn't include them. :|
#include <linux/limits.h>
#include <sys/types.h>

#include <rte_common.h>
#include <rte_config.h>
#include <rte_ethdev.h>

#include <cmdline_parse_etheraddr.h>
#include <cmdline_parse_ipaddr.h>

#include "lib/nf_util.h"
#include "lib/nf_log.h"

const uint16_t DEFAULT_LAN = 1;
const uint16_t DEFAULT_WAN = 0;
const uint64_t DEFAULT_RATE = 1000000; // 1MB/s
const uint64_t DEFAULT_BURST = 100000; // 100kB
const uint32_t DEFAULT_CAPACITY = 128; // IPs

#define PARSE_ERROR(format, ...)          \
  policer_config_cmdline_print_usage();   \
  fprintf(stderr, format, ##__VA_ARGS__); \
  exit(EXIT_FAILURE);

void policer_config_init(struct policer_config *config, int argc, char **argv) {
  // Set the default values
  config->lan_device = DEFAULT_LAN;
  config->wan_device = DEFAULT_WAN;
  config->rate = DEFAULT_RATE;             // B/s
  config->burst = DEFAULT_BURST;           // B
  config->dyn_capacity = DEFAULT_CAPACITY; // MAC addresses

  unsigned nb_devices = rte_eth_dev_count_avail();

  struct option long_options[] = { { "lan-dev", required_argument, NULL, 'l' },
                                   { "wan", required_argument, NULL, 'w' },
                                   { "rate", required_argument, NULL, 'r' },
                                   { "burst", required_argument, NULL, 'b' },
                                   { "capacity", required_argument, NULL, 'c' },
                                  {"eth-dest",		required_argument,	NULL, 'm'},
                                   { NULL, 0, NULL, 0 } };

  // Set the devices' own MACs and driver names
	for (uint16_t device = 0; device < nb_devices; device++) {
		rte_eth_macaddr_get(device, &(config->device_macs[device]));
	}

  int opt;
  while ((opt = getopt_long(argc, argv, "l:w:r:b:c:m:", long_options, NULL)) !=
         EOF) {
    unsigned device;
    switch (opt) {
      case 'l':
        config->lan_device = nf_util_parse_int(optarg, "lan", 10, '\0');
        if (config->lan_device < 0 || config->lan_device >= nb_devices) {
          PARSE_ERROR("Invalid LAN device.\n");
        }
        break;

      case 'w':
        config->wan_device = nf_util_parse_int(optarg, "wan", 10, '\0');
        if (config->wan_device < 0 || config->wan_device >= nb_devices) {
          PARSE_ERROR("Invalid WAN device.\n");
        }
        break;

      case 'r':
        config->rate = nf_util_parse_int(optarg, "rate", 10, '\0');
        if (config->rate == 0) {
          PARSE_ERROR("Policer rate must be strictly positive.\n");
        }
        break;

      case 'b':
        config->burst = nf_util_parse_int(optarg, "burst", 10, '\0');
        if (config->burst == 0) {
          PARSE_ERROR("Policer burst size must be strictly positive.\n");
        }
        break;

      case 'c':
        config->dyn_capacity = nf_util_parse_int(optarg, "capacity", 10, '\0');
        if (config->dyn_capacity <= 0) {
          PARSE_ERROR("Flow table size must be strictly positive.\n");
        }
        break;
      			
      case 'm':
				device = nf_util_parse_int(optarg, "eth-dest device", 10, ',');
				if (device >= nb_devices) {
					PARSE_ERROR("eth-dest: device %d >= nb_devices (%d)\n", device, nb_devices);
				}

				optarg += 2;
				if (cmdline_parse_etheraddr(NULL, optarg, &(config->endpoint_macs[device]), sizeof(int64_t)) < 0) {
					PARSE_ERROR("Invalid MAC address: %s\n", optarg);
				}
				break;

      default:
        PARSE_ERROR("Unknown option %c", opt);
    }
  }

  // Reset getopt
  optind = 1;
}

void policer_config_cmdline_print_usage(void) {
  NF_INFO("Usage:\n"
          "[DPDK EAL options] --\n"
          "\t--lan-dev <device>: LAN device,"
          " default: %" PRIu16 ".\n"
          "\t--wan <device>: WAN device,"
          " default: %" PRIu16 ".\n"
          "\t--rate <rate>: policer rate in bytes/s,"
          " default: %" PRIu64 ".\n"
          "\t--burst <size>: policer burst size in bytes,"
          " default: %" PRIu64 ".\n"
          "\t--capacity <n>: policer table capacity,"
          " default: %" PRIu32 ".\n"
          "\t--eth-dest <device>,<mac>: MAC address of the endpoint linked to a device.\n",
          DEFAULT_LAN, DEFAULT_WAN, DEFAULT_RATE, DEFAULT_BURST,
          DEFAULT_CAPACITY);
}

void policer_print_config(struct policer_config *config) {
  NF_INFO("\n--- Policer Config ---\n");

  NF_INFO("LAN Device: %" PRIu16, config->lan_device);
  NF_INFO("WAN Device: %" PRIu16, config->wan_device);
  NF_INFO("Rate: %" PRIu64, config->rate);
  NF_INFO("Burst: %" PRIu64, config->burst);
  NF_INFO("Capacity: %" PRIu16, config->dyn_capacity);

  uint16_t nb_devices = rte_eth_dev_count_avail();
	for (uint16_t dev = 0; dev < nb_devices; dev++) {
		char* dev_mac_str = nf_mac_to_str(&(config->device_macs[dev]));
		char* end_mac_str = nf_mac_to_str(&(config->endpoint_macs[dev]));

		NF_INFO("Device %" PRIu16 " own-mac: %s, end-mac: %s", dev, dev_mac_str, end_mac_str);

		free(dev_mac_str);
		free(end_mac_str);
	}

  NF_INFO("\n--- ------ ------ ---\n");
}
