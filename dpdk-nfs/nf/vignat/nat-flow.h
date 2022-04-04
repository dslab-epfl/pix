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

struct FlowId {
  uint16_t src_port;
  uint16_t dst_port;
  uint32_t src_ip;
  uint32_t dst_ip;
  uint16_t internal_device;
  uint8_t protocol;
};

int FlowId_hash(void *obj);

bool FlowId_eq(void *a, void *b);

void FlowId_allocate(void *obj);

void log_FlowId(struct FlowId *obj);

#ifdef KLEE_VERIFICATION
#include "lib/stubs/containers/str-descr.h"
#include <klee/klee.h>

#endif // KLEE_VERIFICATION

#endif //_FLOW_H_INCLUDED_
