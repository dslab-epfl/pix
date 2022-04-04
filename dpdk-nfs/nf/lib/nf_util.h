#pragma once

#include <inttypes.h>

// rte_mbuf
struct rte_mbuf;

// rte_ether
struct ether_addr;
struct ether_hdr;
struct arp_hdr;
struct icmp_hdr;

// rte_ip
struct ipv4_hdr;

// A header for TCP or UDP packets, containing common data.
// (This is used to point into DPDK data structures!)
struct tcpudp_hdr {
  uint16_t src_port;
  uint16_t dst_port;
} __attribute__((__packed__));

struct ether_hdr *nf_get_mbuf_ether_header(struct rte_mbuf *mbuf);

struct arp_hdr *nf_get_mbuf_arp_header(struct rte_mbuf *mbuf);

// TODO for consistency it'd be nice if this took an ether_hdr as argument, or
// if they all took rte_mbuf
struct ipv4_hdr *nf_get_mbuf_ipv4_header(struct rte_mbuf *mbuf);

struct tcpudp_hdr *nf_get_ipv4_tcpudp_header(struct ipv4_hdr *header);

struct icmp_hdr *nf_get_ipv4_icmp_header(struct ipv4_hdr *header);

void nf_set_ipv4_checksum(struct ipv4_hdr *header);

uintmax_t nf_util_parse_int(const char *str, const char *name, int base,
                            char next);
char *nf_mac_to_str(struct ether_addr *addr);

char *nf_ipv4_to_str(uint32_t addr);

#ifdef KLEE_VERIFICATION
#define VIGOR_TAG(name, value)                                                 \
  do {                                                                         \
    static const char *name;                                                   \
    klee_make_symbolic(&name, sizeof(const char *), "vigor_tag_" #name);       \
    name = #value;                                                             \
  } while (0)
#else
#define VIGOR_TAG(name, value) TRAFFIC_CLASS(value)
#endif

// KLEE doesn't tolerate && in a klee_assume (see klee/klee#809),
// so we replace them with & during symbex but interpret them as && in the
// validator
#ifdef KLEE_VERIFICATION
#define AND &
#else // KLEE_VERIFICATION
#define AND &&
#endif // KLEE_VERIFICATION

#ifdef KLEE_VERIFICATION
#include <klee/klee.h>
static __attribute__((noinline)) void concretize_devices(uint16_t *device, uint16_t count) {
  klee_assert(*device >= 0);
  klee_assert(*device < count);

  for (unsigned d = 0; d < count; d++)
    if (*device == d) {
      *device = d;
      break;
    }
}
#else
static __attribute__((noinline)) void concretize_devices(uint16_t *device, uint16_t count) {
  (void)(device);
  (void)(count);
}
#endif

/* For perf clarity testing */
/* This conditional macro expansion was obtained from
 * https://stackoverflow.com/questions/11632219/c-preprocessor-macro-specialisation-based-on-an-argument
 */
#define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)
#define PRIMITIVE_CAT(a, ...) a##__VA_ARGS__

#define IIF(c) PRIMITIVE_CAT(IIF_, c)
#define IIF_0(t, ...) __VA_ARGS__
#define IIF_1(t, ...) t

#define PROBE(x) x, 1

#define CHECK(...) CHECK_N(__VA_ARGS__, 0)
#define CHECK_N(x, n, ...) n

#define TC_ENABLED_PROBE(class)                                                \
  TC_ENABLED_PROBE_PROXY(                                                      \
      ENABLE_TC_##class) // concatenate prefix with class name
#define TC_ENABLED_PROBE_PROXY(...)                                            \
  TC_ENABLED_PROBE_PRIMITIVE(__VA_ARGS__) // expand arguments
#define TC_ENABLED_PROBE_PRIMITIVE(x) TC_ENABLED_PROBE_COMBINE_ x // merge
#define TC_ENABLED_PROBE_COMBINE_(...)                                         \
  PROBE(~) // if merge successful, expand to probe

#define IS_TC_ENABLED(class) CHECK(TC_ENABLED_PROBE(class))

#ifdef CLARITY_TEST
#include <assert.h>
#define TRAFFIC_CLASS(class)                                                   \
  IIF(IS_TC_ENABLED(class))                                                    \
  (/* Do nothing */, assert(0 && "Invalid input for perf clarity test");)

#elif defined(DUMP_LATENCY) 
extern int TRAFFIC_CLASS;
#define TRAFFIC_CLASS(class)                                                   \
  NF_TRAFFIC_CLASS = class;                                                    \
  TRAFFIC_CLASS = NF_TRAFFIC_CLASS
#else
#define TRAFFIC_CLASS(class) /* nothing */
#endif

#ifdef KLEE_VERIFICATION
#define DS_INIT(ds_type, ptr, id, data_type)                                   \
  ds_type##_set_id(ptr, id, data_type);
#else
#define DS_INIT(ds_type, ptr, id, data_type)
#endif // KLEE_VERIFICATION