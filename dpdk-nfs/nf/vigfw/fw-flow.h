#ifndef _FLOW_H_INCLUDED_
#define _FLOW_H_INCLUDED_
#include "lib/ignore.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef KLEE_VERIFICATION
#define AND &
#else // KLEE_VERIFICATION
#define AND &&
#endif // KLEE_VERIFICATION

struct Flow {
  uint16_t src_port;
  uint16_t dst_port;
  uint32_t src_ip;
  uint32_t dst_ip;
  uint8_t protocol;
};

int fw_flow_hash(void *obj);

bool fw_flow_eq(void *a, void *b);

void fw_flow_allocate(void *obj);

bool fw_flow_consistency(void* flow, void* unused);

#ifdef KLEE_VERIFICATION
#include "lib/stubs/containers/str-descr.h"
#include <klee/klee.h>

#endif // KLEE_VERIFICATION

#endif //_FLOW_H_INCLUDED_
