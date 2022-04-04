#include "nat-flow.h"
#include "limits.h"


bool FlowId_eq(void* a, void* b)
{
  struct FlowId* id1 = (struct FlowId*) a;
  struct FlowId* id2 = (struct FlowId*) b;
  return (id1->src_port == id2->src_port)
     AND (id1->dst_port == id2->dst_port)
     AND (id1->src_ip == id2->src_ip)
     AND (id1->dst_ip == id2->dst_ip)
     AND (id1->internal_device == id2->internal_device)
     AND (id1->protocol == id2->protocol);
}


void FlowId_allocate(void* obj)
{
  IGNORE(obj);
}

#ifdef KLEE_VERIFICATION
int FlowId_hash(void* obj)
{
  klee_trace_ret();
  klee_trace_param_tagged_ptr(obj, sizeof(struct FlowId),
                             "obj", "FlowId", TD_BOTH);
  return klee_int("FlowId_hash");}

#else//KLEE_VERIFICATION

int FlowId_hash(void* obj)
{
  struct FlowId* id = (struct FlowId*) obj;

  long long hash = 0;
  hash += id->src_port;
  hash *= 31;

  hash += id->dst_port;
  hash *= 31;

  hash += id->src_ip;
  hash *= 31;

  hash += id->dst_ip;
  hash *= 31;

  hash += id->internal_device;
  hash *= 31;

  hash += id->protocol;

  hash = hash % INT_MAX;

  return (int)hash;
}

#endif//KLEE_VERIFICATION

#ifdef ENABLE_LOG
#include "lib/nf_log.h"
void log_FlowId(struct FlowId* obj)
{
  NF_DEBUG("{");
  NF_DEBUG("src_port: %d", obj->src_port);
  NF_DEBUG("dst_port: %d", obj->dst_port);
  NF_DEBUG("src_ip: %d", obj->src_ip);
  NF_DEBUG("dst_ip: %d", obj->dst_ip);
  NF_DEBUG("internal_device: %d", obj->internal_device);
  NF_DEBUG("protocol: %d", obj->protocol);
  NF_DEBUG("}");
}

#  else//ENABLE_LOG
void log_FlowId(struct FlowId* obj)
{
  IGNORE(obj);
}
#endif//ENABLE_LOG

