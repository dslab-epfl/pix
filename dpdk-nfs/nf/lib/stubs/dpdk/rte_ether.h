// used with VeriFast, no pragma
#ifndef RTE_ETHER_H
#define RTE_ETHER_H

#include <stdint.h>
#include <stdio.h>


#define ETHER_TYPE_IPv4 0x0800
#define ETHER_MAX_LEN 1518


struct ether_addr {
// Inline the array for convenience in proofs
//	uint8_t addr_bytes[6];
	uint8_t a;
	uint8_t b;
	uint8_t c;
	uint8_t d;
	uint8_t e;
	uint8_t f;
};

struct ether_hdr {
	struct ether_addr d_addr;
	struct ether_addr s_addr;
	uint16_t ether_type;
};


static void
ether_format_addr(char *buf, uint16_t size, const struct ether_addr *eth_addr)
{
	snprintf(buf, size, "%02X:%02X:%02X:%02X:%02X:%02X",
		eth_addr->a,
		eth_addr->b,
		eth_addr->c,
		eth_addr->d,
		eth_addr->e,
		eth_addr->f);
}

/* Ethernet frame types */
#define ETHER_TYPE_IPv4 0x0800 /**< IPv4 Protocol. */
#define ETHER_TYPE_IPv6 0x86DD /**< IPv6 Protocol. */
#define ETHER_TYPE_ARP  0x0806 /**< Arp Protocol. */
#define ETHER_TYPE_RARP 0x8035 /**< Reverse Arp Protocol. */
#define ETHER_TYPE_VLAN 0x8100 /**< IEEE 802.1Q VLAN tagging. */
#define ETHER_TYPE_QINQ 0x88A8 /**< IEEE 802.1ad QinQ tagging. */
#define ETHER_TYPE_ETAG 0x893F /**< IEEE 802.1BR E-Tag. */
#define ETHER_TYPE_1588 0x88F7 /**< IEEE 802.1AS 1588 Precise Time Protocol. */
#define ETHER_TYPE_SLOW 0x8809 /**< Slow protocols (LACP and Marker). */
#define ETHER_TYPE_TEB  0x6558 /**< Transparent Ethernet Bridging. */
#define ETHER_TYPE_LLDP 0x88CC /**< LLDP Protocol. */
#define ETHER_TYPE_MPLS 0x8847 /**< MPLS ethertype. */
#define ETHER_TYPE_MPLSM 0x8848 /**< MPLS multicast ethertype. */

#endif
