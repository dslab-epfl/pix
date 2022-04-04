// DPDK requires these but doesn't include them. :|
#include <linux/limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

#include <stdlib.h>
#include <string.h>

#include <rte_common.h>
#include <rte_ethdev.h>
#include <rte_ether.h>
#include <rte_ip.h>
#include <rte_mbuf.h>

#include "policer_config.h"
#include "policer_state.h"
#include "lib/nf_forward.h"
#include "lib/nf_log.h"
#include "lib/nf_util.h"
#include "lib/expirator.h"

struct policer_config config;

struct State *flowtable;

int policer_expire_entries(time_t time) {
  assert(time >= 0); // we don't support the past
  assert(sizeof(time_t) <= sizeof(uint64_t));
  uint64_t time_u = (uint64_t)time; // OK because of the two asserts
  time_t last_time =
      time_u - config.burst/config.rate; 

  return expire_items_single_map(flowtable->heap, flowtable->fv,
                                 flowtable->fm, last_time);
}

bool policer_check_tb(uint32_t dst, uint16_t size, time_t time) {
  int index = -1;
  int present = map_get(flowtable->fm, &dst, &index);
  if (present) {
    dchain_rejuvenate_index(flowtable->heap, index, time);

    struct Bucket *value = 0;
    vector_borrow(flowtable->fv, index, (void **)&value);

    assert(0 <= time);
    uint64_t time_u = (uint64_t)time;
    assert(value->bucket_time >= 0);
    uint64_t time_diff = time_u - value->bucket_time;
    if (time_diff <
        config.burst * VIGOR_TIME_SECONDS_MULTIPLIER / config.rate) {
      uint64_t added_tokens =
          (time_diff * config.rate) / VIGOR_TIME_SECONDS_MULTIPLIER;
      value->bucket_size += added_tokens;
      if (value->bucket_size > config.burst) {
        value->bucket_size = config.burst;
      }
    } else {
      value->bucket_size = config.burst;
    }
    value->bucket_time = time_u;

    bool fwd = false;
    if (value->bucket_size > size) {
      value->bucket_size -= size;
      fwd = true;
    }

    vector_return(flowtable->fv, index, value);

    return fwd;
  } else {
    if (size > config.burst) {
      NF_DEBUG("  Unknown flow with packet larger than burst size. Dropping.");
      return false;
    }

    int allocated =
        dchain_allocate_new_index(flowtable->heap, &index, time);
    if (!allocated) {
      NF_DEBUG("No more space in the policer table");
      return false;
    }
    uint32_t *key;
    struct Bucket *value = 0;
    vector_borrow(flowtable->fk, index, (void**)&key);
    vector_borrow(flowtable->fv, index, (void **)&value);
    *key = dst;
    value->bucket_size = config.burst - size;
    value->bucket_time = time;
    map_put(flowtable->fm, key, index);
    vector_return(flowtable->fk, index, key);
    vector_return(flowtable->fv, index, value);
    NF_DEBUG("  New flow. Forwarding.");
    return true;
  }
}

void nf_core_init() {
  unsigned capacity = config.dyn_capacity;
  flowtable = alloc_state(capacity);
  if (flowtable == NULL) {
    rte_exit(EXIT_FAILURE, "Could not allocate flow table");}
}

int nf_core_process(struct rte_mbuf* mbuf,  time_t now) {

  const int in_port = mbuf->port;
  
  struct ether_hdr *ether_header = nf_get_mbuf_ether_header(mbuf);

  struct ipv4_hdr *ipv4_header = nf_get_mbuf_ipv4_header(mbuf);
  if (ipv4_header == NULL) {
    NF_DEBUG("Not IPv4, dropping");
    VIGOR_TAG(TRAFFIC_CLASS, INVALID);
    return mbuf->port;
  }

  policer_expire_entries(now);
  if (in_port == config.lan_device) {
    VIGOR_TAG(TRAFFIC_CLASS, INTERNAL);
    // Simply forward outgoing packets.
    return config.wan_device;
  } else if (in_port == config.wan_device) {
    VIGOR_TAG(TRAFFIC_CLASS, EXTERNAL);
    // Police incoming packets.
    bool fwd = policer_check_tb(ipv4_header->dst_addr, mbuf->pkt_len, now);
    if (fwd) {
      NF_DEBUG("Incoming packet within policed rate. Forwarding.");
      VIGOR_TAG(TRAFFIC_CLASS, PERMITTED);
      return config.lan_device;
    } else {
      NF_DEBUG("Incoming packet outside of policed rate. Dropping.");
      VIGOR_TAG(TRAFFIC_CLASS, RATE_LIMITED);
      return config.wan_device;
    }
  } else {
    // Drop any other packets.
    NF_DEBUG("Unknown port. Dropping.");
    return in_port;
  }
}

void nf_config_init(int argc, char **argv) {
  policer_config_init(&config, argc, argv);
}

void nf_config_cmdline_print_usage(void) { policer_config_cmdline_print_usage(); }

void nf_print_config() { policer_print_config(&config); }
