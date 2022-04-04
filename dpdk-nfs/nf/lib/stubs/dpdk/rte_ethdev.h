#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include "lib/stubs/core_stub.h"

#include <klee/klee.h>

// TX configuration default values 
#define IXGBE_DEFAULT_TX_FREE_THRESH  32
#define IXGBE_DEFAULT_TX_PTHRESH      32
#define IXGBE_DEFAULT_TX_HTHRESH      0
#define IXGBE_DEFAULT_TX_WTHRESH      0
#define IXGBE_DEFAULT_TX_RSBIT_THRESH 32

#define ETH_TXQ_FLAGS_NOMULTSEGS 0x0001 /**< nb_segs=1 for all mbufs */
#define ETH_TXQ_FLAGS_NOVLANOFFL 0x0100 /**< disable VLAN offload */
#define ETH_TXQ_FLAGS_NOXSUMSCTP 0x0200 /**< disable SCTP checksum offload */
#define ETH_TXQ_FLAGS_NOXSUMUDP  0x0400 /**< disable UDP checksum offload */
#define ETH_TXQ_FLAGS_NOXSUMTCP  0x0800 /**< disable TCP checksum offload */
#define ETH_TXQ_FLAGS_NOOFFLOADS \
		(ETH_TXQ_FLAGS_NOVLANOFFL | ETH_TXQ_FLAGS_NOXSUMSCTP | \
		 ETH_TXQ_FLAGS_NOXSUMUDP  | ETH_TXQ_FLAGS_NOXSUMTCP)

#ifdef STUB_DEVICES_COUNT
#define STUB_DPDK_DEVICES_COUNT STUB_DEVICES_COUNT
#else
#define STUB_DPDK_DEVICES_COUNT 2
#endif


struct rte_eth_link {
	uint32_t link_speed;
	uint16_t link_duplex  : 1;
	uint16_t link_autoneg : 1;
	uint16_t link_status  : 1;
};

/**
 * A structure used to configure the ring threshold registers of an RX/TX
 * queue for an Ethernet port.
 */
struct rte_eth_thresh {
	uint8_t pthresh; /**< Ring prefetch threshold. */
	uint8_t hthresh; /**< Ring host threshold. */
	uint8_t wthresh; /**< Ring writeback threshold. */
};

struct rte_eth_conf { /* Nothing */ };
struct rte_eth_rxconf {
	uint16_t rx_free_thresh;
	// we don't care about other members
};
/**
 * A structure used to configure a TX ring of an Ethernet port.
 */
struct rte_eth_txconf {
	struct rte_eth_thresh tx_thresh; /**< TX ring threshold registers. */
	uint16_t tx_rs_thresh; /**< Drives the setting of RS bit on TXDs. */
	uint16_t tx_free_thresh; /**< Start freeing TX buffers if there are
				      less free descriptors than this value. */

	uint32_t txq_flags; /**< Set flags for the Tx queue */
	uint8_t tx_deferred_start; /**< Do not start queue with rte_eth_dev_start(). */
	/**
	 * Per-queue Tx offloads to be set  using DEV_TX_OFFLOAD_* flags.
	 * Only offloads set on tx_queue_offload_capa or tx_offload_capa
	 * fields on rte_eth_dev_info structure are allowed to be set.
	 */
	uint64_t offloads;
};


// Sanity checks
// Documentation of rte_ethdev indicates the configure/tx/rx/started order
static bool devices_configured[STUB_DPDK_DEVICES_COUNT];
static bool devices_tx_setup[STUB_DPDK_DEVICES_COUNT];
static bool devices_rx_setup[STUB_DPDK_DEVICES_COUNT];
static bool devices_started[STUB_DPDK_DEVICES_COUNT];
static bool devices_promiscuous[STUB_DPDK_DEVICES_COUNT];

// To allocate mbufs
static struct rte_mempool* devices_rx_mempool[STUB_DPDK_DEVICES_COUNT];


static inline uint16_t
rte_eth_dev_count_avail(void)
{
	return STUB_DPDK_DEVICES_COUNT;
}

static inline int
rte_eth_dev_configure(uint16_t port_id, uint16_t nb_rx_queue, uint16_t nb_tx_queue,
			const struct rte_eth_conf* eth_conf)
{
	klee_assert(!devices_configured[port_id]);
	klee_assert(nb_rx_queue == 1); // we only support that
	klee_assert(nb_tx_queue == 1); // same
	// TODO somehow semantically check eth_conf?

	devices_configured[port_id] = true;
	return 0;
}

static inline int
rte_eth_tx_queue_setup(uint16_t port_id, uint16_t tx_queue_id,
			uint16_t nb_tx_desc, unsigned int socket_id,
			const struct rte_eth_txconf* tx_conf)
{
	klee_assert(devices_configured[port_id]);
	klee_assert(!devices_tx_setup[port_id]);
	klee_assert(tx_queue_id == 0); // we only support that
	klee_assert(socket_id == 0); // same
	if (tx_conf != NULL ) {
		klee_assert(tx_conf->tx_thresh.pthresh == IXGBE_DEFAULT_TX_PTHRESH);
		klee_assert(tx_conf->tx_thresh.hthresh == IXGBE_DEFAULT_TX_HTHRESH);
		klee_assert(tx_conf->tx_thresh.wthresh == IXGBE_DEFAULT_TX_WTHRESH);
		klee_assert(tx_conf->tx_free_thresh == 1);
		klee_assert(tx_conf->tx_rs_thresh == 1);
		klee_assert(tx_conf->txq_flags == 
                            (ETH_TXQ_FLAGS_NOMULTSEGS | ETH_TXQ_FLAGS_NOOFFLOADS));
	}

	devices_tx_setup[port_id] = true;
	return 0;
}

static inline int
rte_eth_rx_queue_setup(uint16_t port_id, uint16_t rx_queue_id, uint16_t nb_rx_desc,
			unsigned int socket_id, const struct rte_eth_rxconf *rx_conf,
			struct rte_mempool *mb_pool)
{
	klee_assert(devices_tx_setup[port_id]);
	klee_assert(!devices_rx_setup[port_id]);
	klee_assert(rx_queue_id == 0); // we only support that
	klee_assert(socket_id == 0); // same
	klee_assert(mb_pool != NULL);
	// TODO semantic checks for rx_conf? since we need it for the hardware verif

	devices_rx_setup[port_id] = true;
	devices_rx_mempool[port_id] = mb_pool;
	return 0;
}

static inline int
rte_eth_dev_start(uint16_t port_id)
{
	klee_assert(devices_rx_setup[port_id]);
	klee_assert(!devices_started[port_id]);

	devices_started[port_id] = true;
	return 0;
}

static inline void
rte_eth_promiscuous_enable(uint16_t port_id)
{
	klee_assert(!devices_promiscuous[port_id]);
	devices_promiscuous[port_id] = true;
}

static inline int
rte_eth_promiscuous_get(uint16_t port_id)
{
	return devices_promiscuous[port_id] ? 1 : 0;
}

static inline int
rte_eth_dev_socket_id(uint16_t port_id)
{
	klee_assert(port_id < STUB_DPDK_DEVICES_COUNT);

	return 0;
}

static inline void
rte_eth_macaddr_get(uint16_t port_id, struct ether_addr *mac_addr)
{
	// TODO?
}

static uint16_t __attribute__((noinline))
rte_eth_rx_burst(uint16_t port_id, uint16_t queue_id,
		 struct rte_mbuf **rx_pkts, const uint16_t nb_pkts)
{
	klee_assert(devices_started[port_id]);
	klee_assert(queue_id == 0); // we only support that
	klee_assert(nb_pkts == 1); // same

	if (klee_int("received") == 0) {
		return 0;
	}

	struct rte_mempool* pool = devices_rx_mempool[port_id];
	stub_core_mbuf_create(port_id, pool, rx_pkts);
	stub_core_trace_rx(rx_pkts);

	return 1;
}

static uint16_t __attribute__((noinline))
rte_eth_tx_burst(uint16_t port_id, uint16_t queue_id,
		 struct rte_mbuf **tx_pkts, uint16_t nb_pkts)
{
	klee_assert(devices_started[port_id]);
	klee_assert(queue_id == 0); // we only support that
	klee_assert(nb_pkts == 1); // same

	uint8_t ret = stub_core_trace_tx(*tx_pkts, port_id);
	if (ret == 0) {
		return 0;
	}

	stub_core_mbuf_free(*tx_pkts);
	return 1;
}
