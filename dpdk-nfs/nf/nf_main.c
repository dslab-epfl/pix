#ifdef KLEE_VERIFICATION
#include "lib/stubs/driver_stub.h"
#include "lib/stubs/hardware_stub.h"
#include "lib/stubs/time_stub_control.h"
#include <klee/klee.h>
#endif

#include <inttypes.h>
#include <stdio.h>
#include <assert.h>

//#define REPLAY
// TX configuration default values
#define IXGBE_DEFAULT_TX_FREE_THRESH 32
#define IXGBE_DEFAULT_TX_PTHRESH 32
#define IXGBE_DEFAULT_TX_HTHRESH 0
#define IXGBE_DEFAULT_TX_WTHRESH 0
#define IXGBE_DEFAULT_TX_RSBIT_THRESH 32

#ifdef FREUD
#define MAX_PKTS 500
#endif // FREUD

#ifdef KLEE_VERIFICATION
#define VIGOR_LOOP_BEGIN                                                       \
  unsigned _vigor_lcore_id = rte_lcore_id();                                   \
  time_t _vigor_start_time = start_time();                                     \
  int _vigor_loop_termination = klee_int("loop_termination");                  \
  unsigned VIGOR_DEVICES_COUNT;                                                \
  klee_possibly_havoc(&VIGOR_DEVICES_COUNT, sizeof(VIGOR_DEVICES_COUNT),       \
                      "VIGOR_DEVICES_COUNT");                                  \
  time_t VIGOR_NOW;                                                            \
  klee_possibly_havoc(&VIGOR_NOW, sizeof(VIGOR_NOW), "VIGOR_NOW");             \
  unsigned VIGOR_DEVICE;                                                       \
  klee_possibly_havoc(&VIGOR_DEVICE, sizeof(VIGOR_DEVICE), "VIGOR_DEVICE");    \
  unsigned _d;                                                                 \
  klee_possibly_havoc(&_d, sizeof(_d), "_d");                                  \
  while (klee_induce_invariants() & _vigor_loop_termination) {                 \
    nf_loop_iteration_border(_vigor_lcore_id, _vigor_start_time);              \
    VIGOR_NOW = current_time();                                                \
    /* concretize the device to avoid leaking symbols into DPDK */             \
    VIGOR_DEVICES_COUNT = rte_eth_dev_count_avail();                           \
    VIGOR_DEVICE = klee_range(0, VIGOR_DEVICES_COUNT, "VIGOR_DEVICE");         \
    for (_d = 0; _d < VIGOR_DEVICES_COUNT; _d++)                               \
      if (VIGOR_DEVICE == _d) {                                                \
        VIGOR_DEVICE = _d;                                                     \
        break;                                                                 \
      }                                                                        \
    stub_hardware_receive_packet(VIGOR_DEVICE);
#ifdef REPLAY
#define VIGOR_LOOP_END                                                         \
  stub_hardware_reset_receive(VIGOR_DEVICE);                                   \
  nf_loop_iteration_border(_vigor_lcore_id, VIGOR_NOW);                        \
  if (klee_int("tired")) {                                                     \
    exit(0);                                                                   \
  }                                                                            \
  }
#else // REPLAY
#define VIGOR_LOOP_END                                                         \
  stub_hardware_reset_receive(VIGOR_DEVICE);                                   \
  nf_loop_iteration_border(_vigor_lcore_id, VIGOR_NOW);                        \
  }
#endif // REPLAY
#else  // KLEE_VERIFICATION
#define VIGOR_LOOP_BEGIN                                                       \
  while (1) {                                                                  \
    time_t VIGOR_NOW = current_time();                                         \
    unsigned VIGOR_DEVICES_COUNT = rte_eth_dev_count_avail();                  \
    for (uint16_t VIGOR_DEVICE = 0; VIGOR_DEVICE < VIGOR_DEVICES_COUNT;        \
         VIGOR_DEVICE++) {
#define VIGOR_LOOP_END                                                         \
  }                                                                            \
  }
#endif // KLEE_VERIFICATION

// DPDK uses these but doesn't include them. :|
#include <linux/limits.h>
#include <sys/types.h>

#include <stdlib.h>
#include <string.h>

#include <rte_common.h>
#include <rte_eal.h>
#include <rte_errno.h>
#include <rte_ethdev.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>

#include "lib/nf_forward.h"
#include "lib/nf_log.h"
#include "lib/nf_time.h"
#include "lib/nf_util.h"
#include "lib/nf_perf_log.h"


// Number of RX/TX queues
static const uint16_t RX_QUEUES_COUNT = 1;
static const uint16_t TX_QUEUES_COUNT = 1;

#define batch_param 1000000
#define MMAP_NUM_ENTRIES 1000000 
#define CPU_HZ    3292060000 // Change this for different computers. (cat /proc/cpuinfo)
#define PAGE_SIZE   4096

#if (defined DUMP_LATENCY)
#include "lib/nf_log.h"
#include "x86intrin.h"
#include <time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
int TRAFFIC_CLASS;
long long* mmap_file;
long mmap_file_cntr = 0;
// Returns time in nanoseconds scale.
static __inline__ long rdtsc(void) {
      unsigned a, d;
      asm volatile("rdtsc" : "=a" (a), "=d" (d));
      return (long)((double)(((long)a) | (((long)d) << 32)) / (double)CPU_HZ * 1000000000);
}
#endif // DUMP_LATENCY

// --- Static config ---
// TODO see remark in lcore_main
// Size of batches to receive; trade-off between latency and throughput
// Can be overriden at compile time
//#ifndef BATCH_SIZE
// static const uint16_t BATCH_SIZE = 32;
//#endif

// Queue sizes for receiving/transmitting packets
// NOT powers of 2 so that ixgbe doesn't use vector stuff
// but they have to be multiples of 8, and at least 32, otherwise the driver
// refuses
static const uint16_t RX_QUEUE_SIZE = 96;
static const uint16_t TX_QUEUE_SIZE = 96;

// Clone pool for flood()
static struct rte_mempool *clone_pool;

// Buffer count for mempools
static const unsigned MEMPOOL_BUFFER_COUNT = 256;

// --- Initialization ---
static int nf_init_device(uint16_t device, struct rte_mempool *mbuf_pool) {
  int retval;

  // device_conf passed to rte_eth_dev_configure cannot be NULL
  struct rte_eth_conf device_conf;
  memset(&device_conf, 0, sizeof(struct rte_eth_conf));

  // Configure the device
  retval = rte_eth_dev_configure(device, RX_QUEUES_COUNT, TX_QUEUES_COUNT,
                                 &device_conf);
  if (retval != 0) {
    return retval;
  }

  struct rte_eth_txconf *tx_confp = NULL;
  // This config breaks verify-hardware. however it is nessary to ensure,
  // we avoid batch buffer recycling on some packets. It enforces the dpdk to
  // free a buffer right after a packet is sent (after some warm-up period,
  // like starting from a packet#95).
  // In verify-klee, this config is ignored anyway.
  // In verify-hardware, this config apprently leads to an explosion. Instead,
  // to ensure the state at the end of an iteration (send+receive) matches
  // the one at the beginning, we employ a "hack" -stub_hardware_reset_receive
#ifndef KLEE_VERIFICATION
  // Allocate and set up TX queues
  struct rte_eth_txconf tx_conf;
  tx_conf = (struct rte_eth_txconf){
      .tx_thresh =
          {
              .pthresh = IXGBE_DEFAULT_TX_PTHRESH,
              .hthresh = IXGBE_DEFAULT_TX_HTHRESH,
              .wthresh = IXGBE_DEFAULT_TX_WTHRESH,
          },
      .tx_free_thresh = IXGBE_DEFAULT_TX_FREE_THRESH,
      .tx_rs_thresh = IXGBE_DEFAULT_TX_RSBIT_THRESH,
  };

  tx_conf.tx_free_thresh = 1;
  tx_conf.tx_rs_thresh = 1;
  tx_confp = &tx_conf;
#endif //! KLEE_VERIFICATION

  for (int txq = 0; txq < TX_QUEUES_COUNT; txq++) {
    retval = rte_eth_tx_queue_setup(device, txq, TX_QUEUE_SIZE,
                                    rte_eth_dev_socket_id(device),
                                    NULL // default config
    );
    if (retval != 0) {
      return retval;
    }
  }

  // Allocate and set up RX queues
  for (int rxq = 0; rxq < RX_QUEUES_COUNT; rxq++) {
    retval = rte_eth_rx_queue_setup(device, rxq, RX_QUEUE_SIZE,
                                    rte_eth_dev_socket_id(device),
                                    NULL, // default config
                                    mbuf_pool);
    if (retval != 0) {
      return retval;
    }
  }

  // Start the device
  retval = rte_eth_dev_start(device);
  if (retval != 0) {
    return retval;
  }

  // Enable RX in promiscuous mode, just in case
  rte_eth_promiscuous_enable(device);
  if (rte_eth_promiscuous_get(device) != 1) {
    return retval;
  }

  return 0;
}

// Flood method for the bridge
#ifdef KLEE_VERIFICATION
void flood(struct rte_mbuf *frame, uint16_t skip_device,
           uint16_t nb_devices); // defined in stubs
#else
void flood(struct rte_mbuf *frame, uint16_t skip_device, uint16_t nb_devices) {
  for (uint16_t device = 0; device < nb_devices; device++) {
    if (device == skip_device)
      continue;
    struct rte_mbuf *copy = rte_pktmbuf_clone(frame, clone_pool);
    if (copy == NULL) {
      rte_exit(EXIT_FAILURE, "Cannot clone a frame for flooding");
    }
    uint16_t actual_tx_len = rte_eth_tx_burst(device, 0, &copy, 1);

    if (actual_tx_len == 0) {
      rte_pktmbuf_free(copy);
    }
  }
  rte_pktmbuf_free(frame);
}
#endif //! KLEE_VERIFICATION

// --- Per-core work ---

static void lcore_main(void) {
  // TODO is this check useful?
  for (uint16_t device = 0; device < rte_eth_dev_count_avail(); device++) {
    if (rte_eth_dev_socket_id(device) > 0 &&
        rte_eth_dev_socket_id(device) != (int)rte_socket_id()) {
      NF_INFO("Device %" PRIu8 " is on remote NUMA node to polling thread.",
              device);
    }
  }

  nf_core_init();

  NF_INFO("Core %u forwarding packets.", rte_lcore_id());

#ifdef FREUD
  long long packets_processed = 0;
#endif

  uint16_t actual_rx_len;
  uint16_t dst_device;
  struct rte_mbuf *buf;
  uint16_t actual_tx_len;
#ifdef KLEE_VERIFICATION
  klee_possibly_havoc(&actual_rx_len, sizeof(actual_rx_len), "actual_rx_len");
  klee_possibly_havoc(&dst_device, sizeof(dst_device), "dst_device");
  klee_possibly_havoc(&buf, sizeof(buf), "buf_addr");
  klee_possibly_havoc(&actual_tx_len, sizeof(actual_tx_len), "actual_tx_len");
#endif // KLEE_VERIFICATION

/* Setting up perf measurement code before event loop */
TN_PERF_PAPI_INIT();
/* Event loop begins */
TN_PERF_PAPI_RESET();
  VIGOR_LOOP_BEGIN
  buf = NULL;
  
  actual_rx_len = rte_eth_rx_burst(VIGOR_DEVICE, 0, &buf, 1);

  if (actual_rx_len != 0) {

    // TN_PERF_PAPI_RESET();
    TN_PERF_PAPI_LOG_PKT();

    #ifdef FREUD
      packets_processed++;
    #endif

/* Start measurements */

    #ifdef DUMP_LATENCY
      long long start_time = rdtsc();
    #endif // DUMP_LATENCY
    
    #ifdef DUMP_PERF_VARS
          NF_PERF_DEBUG("New Packet");
    #endif

      dst_device = nf_core_process(&buf[0], VIGOR_NOW);

      /* End measurements */
      // TN_PERF_PAPI_RECORD(1);

    #ifdef DUMP_LATENCY
      long long end_time = rdtsc();
      mmap_file[mmap_file_cntr++] = end_time - start_time;
      if (mmap_file_cntr == MMAP_NUM_ENTRIES) // To Avoid overflow
        mmap_file_cntr = 0;
    #endif // DUMP_LATENCY



    if (dst_device == VIGOR_DEVICE) {
      rte_pktmbuf_free(buf);
    } else if (dst_device == FLOOD_FRAME) {
      flood(buf, VIGOR_DEVICE, VIGOR_DEVICES_COUNT);
    } else {
      actual_tx_len = rte_eth_tx_burst(dst_device, 0, &buf, 1);
      if (actual_tx_len == 0) {
        rte_pktmbuf_free(buf);
      }
    }
  }


#ifdef FREUD
 if (packets_processed == MAX_PKTS){
   return;
 }
#endif // FREUD

#ifdef STOP_ON_RX_0
  else if (VIGOR_DEVICE == 0) {
    return;
  }
#endif
#ifdef STOP_ON_RX_1
  else if (VIGOR_DEVICE == 1) {
    return;
  }
#endif

  VIGOR_LOOP_END
}

// --- Main ---

int main(int argc, char *argv[]) {
  // Initialize the Environment Abstraction Layer (EAL)
  int ret = rte_eal_init(argc, argv);
  if (ret < 0) {
    rte_exit(EXIT_FAILURE, "Error with EAL initialization, ret=%d\n", ret);
  }
  argc -= ret;
  argv += ret;

#ifdef KLEE_VERIFICATION
  // Attach stub driver (note that hardware stub is autodetected, no need to
  // attach)
  stub_driver_attach();
#endif

  // NF-specific config
  nf_config_init(argc, argv);
  nf_print_config();

  // Create a memory pool
  unsigned nb_devices = rte_eth_dev_count_avail();
  struct rte_mempool *mbuf_pool = rte_pktmbuf_pool_create(
      "MEMPOOL",                         // name
      MEMPOOL_BUFFER_COUNT * nb_devices, // #elements
      0, // cache size (per-lcore, not useful in a single-threaded app)
      0, // application private area size
      RTE_MBUF_DEFAULT_BUF_SIZE, // data buffer size
      rte_socket_id()            // socket ID
  );
  if (mbuf_pool == NULL) {
    rte_exit(EXIT_FAILURE, "Cannot create mbuf pool: %s\n",
             rte_strerror(rte_errno));
  }

  // Create another pool for the flood() cloning
  clone_pool =
      rte_pktmbuf_pool_create("clone_pool",         // name
                              MEMPOOL_BUFFER_COUNT, // #elements
                              0, // cache size (same remark as above)
                              0, // application private data size
                              RTE_MBUF_DEFAULT_BUF_SIZE, // data buffer size
                              rte_socket_id()            // socket ID
      );
  if (clone_pool == NULL) {
    rte_exit(EXIT_FAILURE, "Cannot create mbuf clone pool: %s\n",
             rte_strerror(rte_errno));
  }

  // Initialize all devices
  for (uint16_t device = 0; device < nb_devices; device++) {
    ret = nf_init_device(device, mbuf_pool);
    if (ret == 0) {
      NF_INFO("Initialized device %" PRIu8 ".", device);
    } else {
      rte_exit(EXIT_FAILURE, "Cannot init device %" PRIu8 ", ret=%d", device,
               ret);
    }
  }

  // Run!
  // ...in single-threaded mode, that is.

#if (defined DUMP_LATENCY)
  FILE *fp = fopen("latency_log.bin","w+");
  if(fp == 0)
    assert (0 && "fopen failed");
  fseek(fp,(MMAP_NUM_ENTRIES*sizeof(long))-1,SEEK_SET);
  fwrite("", 1, sizeof(char), fp);
  fflush(fp);
  fclose(fp);

  int fd = open("./latency_log.bin", O_RDWR);
  if(fd < 0)
    assert(0 && "open failed");
  mmap_file = (long long *) mmap(NULL, MMAP_NUM_ENTRIES*sizeof(long long), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if(mmap_file == MAP_FAILED)
    assert(0 && "mmap failed");
  close(fd);
  long gap = PAGE_SIZE/sizeof(long long);
  // Touch each page to load them to the TLB.
  for (long i = 0; i < MMAP_NUM_ENTRIES; i += gap)
    mmap_file[i] = 0;
#endif //DUMP_LATENCY 

  lcore_main();

  return 0;
}
