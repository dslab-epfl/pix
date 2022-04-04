// DPDK uses these but doesn't include them. :|
#include <assert.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/time.h>

#include <rte_byteorder.h>
#include <rte_ethdev.h>
#include <rte_ip.h>
#include <rte_mbuf.h>

#include "lib/nat_config.h"
#include "lib/nf_forward.h"
#include "lib/nf_util.h"
#include "lpm/lpm.h"

struct nat_config config;
void *lpm;

enum TrafficClass NF_TRAFFIC_CLASS = UNDEFINED;

void nf_core_init(void) {
  lpm_init("routing-table.pfx2as", &lpm);
  assert(lpm);
}

int nf_core_process(struct rte_mbuf *mbuf, time_t now) {
  // VIGOR_TAG(TRAFFIC_CLASS, PACKET_RECEIVED);
  struct ipv4_hdr *ip_header = nf_get_mbuf_ipv4_header(mbuf);
  if (ip_header == NULL) {
    return mbuf->port;
  }

  uint8_t dst_device = lpm_lookup(lpm, rte_be_to_cpu_32(ip_header->dst_addr));

#ifdef KLEE_VERIFICATION
  // Concretize the device, to avoid symbolic indexing.
  for (uint8_t d = 0; d < rte_eth_dev_count_avail(); ++d) {
    if (dst_device == d) {
      dst_device = d;
    }
  }
#endif

  // L2 forwarding
  struct ether_hdr *ether_header = nf_get_mbuf_ether_header(mbuf);
  ether_header->s_addr = config.device_macs[dst_device];
  ether_header->d_addr = config.endpoint_macs[dst_device];

  return dst_device;
}

void nf_config_init(int argc, char **argv) {
  nat_config_init(&config, argc, argv);
}

void nf_config_cmdline_print_usage(void) { nat_config_cmdline_print_usage(); }

void nf_print_config() { nat_print_config(&config); }

#ifdef KLEE_VERIFICATION
void nf_loop_iteration_border(unsigned lcore_id, time_t time) {}
#endif
