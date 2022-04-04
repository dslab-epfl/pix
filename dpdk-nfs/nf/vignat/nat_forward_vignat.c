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

#include "nat-flow.h"
#include "vignat/nat-flowmanager.h"

#include "lib/nat_config.h"
#include "lib/nf_forward.h"
#include "lib/nf_log.h"
#include "lib/nf_util.h"

struct nat_config config;
struct FlowManager *flow_manager;

enum TrafficClass NF_TRAFFIC_CLASS = UNDEFINED;

void nf_core_init() {
  flow_manager = flow_manager_allocate(
      config.start_port, config.external_addr, config.wan_device,
      config.expiration_time, config.max_flows);

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

    struct FlowId internal_flow;
    if (flow_manager_get_external(flow_manager, tcpudp_header->dst_port, now,
                                  &internal_flow)) {
      NF_DEBUG("Found internal flow.");
      log_FlowId(&internal_flow);

      // VIGOR_TAG(TRAFFIC_CLASS, EXTERNAL_KNOWN);

      ipv4_header->dst_addr = internal_flow.src_ip;
      tcpudp_header->dst_port = internal_flow.src_port;
      dst_device = internal_flow.internal_device;
    } else {
      // VIGOR_TAG(TRAFFIC_CLASS, EXTERNAL_NEW);
      NF_DEBUG("Unknown flow, dropping");
      return in_port;
    }
  } else {
    // VIGOR_TAG(TRAFFIC_CLASS, INTERNAL);

    struct FlowId id = {.src_port = tcpudp_header->src_port,
                        .dst_port = tcpudp_header->dst_port,
                        .src_ip = ipv4_header->src_addr,
                        .dst_ip = ipv4_header->dst_addr,
                        .protocol = ipv4_header->next_proto_id,
                        .internal_device = in_port};

    NF_DEBUG("For id:");
    log_FlowId(&id);

    NF_DEBUG("Device %" PRIu16 " is internal (not %" PRIu16 ")", in_port,
             config.wan_device);

    uint16_t external_port;
    if (!flow_manager_get_internal(flow_manager, &id, now, &external_port)) {
      NF_DEBUG("New flow");

      // VIGOR_TAG(TRAFFIC_CLASS, INTERNAL_NEW);

      if (!flow_manager_allocate_flow(flow_manager, &id, in_port, now,
                                      &external_port)) {
        NF_DEBUG("No space for the flow, dropping");
        // VIGOR_TAG(TRAFFIC_CLASS, INTERNAL_NEW_FULL);
        return in_port;
      }
    } else {
      // VIGOR_TAG(TRAFFIC_CLASS, INTERNAL_KNOWN);
    }

    NF_DEBUG("Forwarding from ext port:%d", external_port);

    ipv4_header->src_addr = config.external_addr;
    tcpudp_header->src_port = external_port;
    dst_device = config.wan_device;
  }

  ether_header->s_addr = config.device_macs[dst_device];
  ether_header->d_addr = config.endpoint_macs[dst_device];
  nf_set_ipv4_checksum(ipv4_header);
  /* Simulating mlx5 NICs */
  // if(config.driver_type != nic_type_net_mlx5){
  //   nf_set_ipv4_checksum(ipv4_header);
  // }

#ifdef KLEE_VERIFICATION
  concretize_devices(&dst_device, rte_eth_dev_count_avail());
#endif

  return dst_device;
}

void nf_config_init(int argc, char **argv) {
  nat_config_init(&config, argc, argv);
}

void nf_config_cmdline_print_usage(void) { nat_config_cmdline_print_usage(); }

void nf_print_config() { nat_print_config(&config); }