// DPDK uses these but doesn't include them. :|
#include <linux/limits.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/time.h>

#include <rte_ethdev.h>
#include <rte_ip.h>
#include <rte_mbuf.h>

#include "lib/nat_config.h"
#include "lib/nf_forward.h"
#include "lib/nf_util.h"
#include "router/router_options.h"

struct nat_config config;

enum TrafficClass NF_TRAFFIC_CLASS = UNDEFINED;

void nf_core_init(void) {}

int nf_core_process(struct rte_mbuf *mbuf, time_t now) {
  // Mark now as unused, we don't care about absolute time here
  (void)now;
  VIGOR_TAG(TRAFFIC_CLASS, PACKET_RECEIVED);

  uint32_t router_ip = 0x12345678;

  // // from https://stackoverflow.com/a/10499119
  // struct timeval tv;
  // uint32_t current_milliseconds_utc;
  // if (gettimeofday(&tv, NULL) == 0) {
  //   current_milliseconds_utc = ((tv.tv_sec % 86400) * 1000 + tv.tv_usec /
  //   1000);
  // } else {
  //   return mbuf->port;
  // }

  struct ether_hdr *ether_header = nf_get_mbuf_ether_header(mbuf);
  struct ipv4_hdr *ip_header = nf_get_mbuf_ipv4_header(mbuf);
  // Are there timestamp options we must respect?
  if (ip_header == NULL) {
    VIGOR_TAG(TRAFFIC_CLASS, INVALID);
    return mbuf->port;
  } else {
    VIGOR_TAG(TRAFFIC_CLASS, VALID);
    if ((ip_header->version_ihl & 0xF) > 5) {
      VIGOR_TAG(TRAFFIC_CLASS, IP_OPT);
      if (!handle_packet_timestamp(ip_header, router_ip, 100)) {
        return mbuf->port;
      }
    }
  }

  uint8_t dst_device = mbuf->port == config.lan_main_device
                           ? config.wan_device
                           : config.lan_main_device;

  // L2 forwarding
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
