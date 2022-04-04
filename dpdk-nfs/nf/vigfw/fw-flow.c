#include "fw-flow.h"
#include "limits.h"


bool fw_flow_eq(void* a, void* b)
{
  struct Flow* id1 = (struct Flow*) a;
  struct Flow* id2 = (struct Flow*) b;
  return (id1->src_port == id2->src_port)
     AND (id1->dst_port == id2->dst_port)
     AND (id1->src_ip == id2->src_ip)
     AND (id1->dst_ip == id2->dst_ip)
     AND (id1->protocol == id2->protocol);
}


void fw_flow_allocate(void* obj)
{
  IGNORE(obj);
}

bool fw_flow_consistency(void* flow, void* unused){
  struct Flow* f = (struct Flow*) flow;
  return f->src_port >=0;
}

int fw_flow_hash(void* obj)
{
  struct Flow* id = (struct Flow*) obj;

  long long hash = 0;
  hash += id->src_port;
  hash *= 31;

  hash += id->dst_port;
  hash *= 31;

  hash += id->src_ip;
  hash *= 31;

  hash += id->dst_ip;
  hash *= 31;

  hash += id->protocol;

  hash = hash % INT_MAX;

  return (int)hash;
}
