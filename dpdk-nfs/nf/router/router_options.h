#pragma once

#include <stdbool.h>

#include <rte_ip.h>


bool
handle_packet_timestamp(struct ipv4_hdr* header, uint32_t router_ip, uint32_t current_milliseconds_utc);
