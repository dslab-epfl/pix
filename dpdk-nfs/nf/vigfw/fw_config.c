#include "fw_config.h"

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


#define PARSE_ERROR(format, ...)          \
  fw_config_cmdline_print_usage();   \
  fprintf(stderr, format, ##__VA_ARGS__); \
  exit(EXIT_FAILURE);

void fw_config_init(struct fw_config *config, int argc, char **argv) {

  unsigned nb_devices = rte_eth_dev_count_avail();

  struct option long_options[] = { { "lan-dev", required_argument, NULL, 'l' },
                                   { "wan", required_argument, NULL, 'w' },
                                   { "capacity", required_argument, NULL, 'c' },
                                   { "expire",	required_argument,	NULL, 't'},
                                   { "eth-dest",		required_argument,	NULL, 'm'},
                                   { NULL, 0, NULL, 0 } };

  // Set the devices' own MACs and driver names
	for (uint16_t device = 0; device < nb_devices; device++) {
		rte_eth_macaddr_get(device, &(config->device_macs[device]));
	}

  int opt;
  while ((opt = getopt_long(argc, argv, "l:w:c:t:m:", long_options, NULL)) !=
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

      case 'c':
        config->capacity = nf_util_parse_int(optarg, "capacity", 10, '\0');
        if (config->capacity <= 0) {
          PARSE_ERROR("Flow table size must be strictly positive.\n");
        }
        break;
      
    	case 't':
				config->expiration_time = nf_util_parse_int(optarg, "exp-time", 10, '\0');
				if (config->expiration_time == 0) {
					PARSE_ERROR("Expiration time must be strictly positive.\n");
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

void fw_config_cmdline_print_usage(void) {
  NF_INFO("Usage:\n"
          "[DPDK EAL options] --\n"
          "\t--lan-dev <device>: LAN device,"
          "\t--wan <device>: WAN device,"
          "\t--capacity <n>: firewall table capacity,"
          "\t--expire <time>: flow expiration time.\n"
          "\t--eth-dest <device>,<mac>: MAC address of the endpoint linked to a device.\n");
}

void fw_print_config(struct fw_config *config) {
  NF_INFO("\n--- Firewall Config ---\n");

  NF_INFO("LAN Device: %" PRIu16, config->lan_device);
  NF_INFO("WAN Device: %" PRIu16, config->wan_device);
  NF_INFO("Capacity: %" PRIu16, config->capacity);
  NF_INFO("Expiration time: %" PRIu32, config->expiration_time);

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
