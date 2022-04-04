// DPDK requires these but doesn't include them. :|
#include <linux/limits.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdlib.h>
#include <string.h>

#include <rte_common.h>
#include <rte_ethdev.h>
#include <rte_ether.h>
#include <rte_ip.h>
#include <rte_mbuf.h>

#include "fw-flow.h"
#include "fw-flowmanager.h"

#include "fw_config.h"
#include "lib/nf_forward.h"
#include "lib/nf_log.h"
#include "lib/nf_util.h"

struct fw_config config;
struct FlowManager *flow_manager;

enum TrafficClass NF_TRAFFIC_CLASS = UNDEFINED;

void nf_core_init() {
  flow_manager = flow_manager_allocate(config.expiration_time, config.capacity);

  if (flow_manager == NULL) {
    rte_exit(EXIT_FAILURE, "Could not allocate flow manager");
  }
}

int nf_core_process(struct rte_mbuf *mbuf, time_t now) {

  // VIGOR_TAG(TRAFFIC_CLASS, PACKET_RECEIVED);
  NF_DEBUG("It is %" PRIu32, now);

  const int in_port = mbuf->port;

  flow_manager_expire(flow_manager, now);
  NF_DEBUG("Flows have been expired");

  struct ether_hdr *ether_header = nf_get_mbuf_ether_header(mbuf);

  struct ipv4_hdr *ipv4_header = nf_get_mbuf_ipv4_header(mbuf);
  if (ipv4_header == NULL) {
    NF_DEBUG("Not IPv4, dropping");
    // VIGOR_TAG(TRAFFIC_CLASS, INVALID);
    return mbuf->port;
  }

  struct tcpudp_hdr *tcpudp_header = nf_get_ipv4_tcpudp_header(ipv4_header);
  if (tcpudp_header == NULL) {
    NF_DEBUG("Not TCP/UDP, dropping");
    // VIGOR_TAG(TRAFFIC_CLASS, INVALID);
    return mbuf->port;
  }

  // VIGOR_TAG(TRAFFIC_CLASS, VALID);

  NF_DEBUG("Forwarding an IPv4 packet on device %" PRIu16, mbuf->port);

  uint16_t dst_device;
  if (in_port == config.wan_device) {
    // VIGOR_TAG(TRAFFIC_CLASS, EXTERNAL);
    NF_DEBUG("Device %" PRIu16 " is external", mbuf->port);

    /* Flip src and destination */
    struct Flow id = {.src_port = tcpudp_header->dst_port,
                        .dst_port = tcpudp_header->src_port,
                        .src_ip = ipv4_header->dst_addr,
                        .dst_ip = ipv4_header->src_addr,
                        .protocol = ipv4_header->next_proto_id};

    if (flow_manager_get(flow_manager, &id, now)) {
        dst_device = config.lan_device;
    } else {
      // VIGOR_TAG(TRAFFIC_CLASS, DROPPED);
      NF_DEBUG("Unknown flow, dropping");
      return in_port;
    }
  } else {
    // VIGOR_TAG(TRAFFIC_CLASS, INTERNAL);
    struct Flow id = {.src_port = tcpudp_header->src_port,
                        .dst_port = tcpudp_header->dst_port,
                        .src_ip = ipv4_header->src_addr,
                        .dst_ip = ipv4_header->dst_addr,
                        .protocol = ipv4_header->next_proto_id};

    NF_DEBUG("Device %" PRIu16 " is internal (not %" PRIu16 ")", in_port,
             config.wan_device);

    uint16_t external_port;
    if (!flow_manager_get(flow_manager, &id, now)) {
      NF_DEBUG("New flow");

      // VIGOR_TAG(TRAFFIC_CLASS, INTERNAL_NEW);
      if (!flow_manager_allocate_flow(flow_manager, &id, now)) {
        NF_DEBUG("No space for the flow, dropping");
        return in_port;
      }
    } else {
      // VIGOR_TAG(TRAFFIC_CLASS, INTERNAL_KNOWN);
    }
    dst_device = config.wan_device;
  }

  ether_header->s_addr = config.device_macs[dst_device];
  ether_header->d_addr = config.endpoint_macs[dst_device];
  return dst_device;
}

void nf_config_init(int argc, char **argv) {
  fw_config_init(&config, argc, argv);
}

void nf_config_cmdline_print_usage(void) { fw_config_cmdline_print_usage(); }

void nf_print_config() { fw_print_config(&config); }