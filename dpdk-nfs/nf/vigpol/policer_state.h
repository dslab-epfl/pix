#ifndef _STATE_H_INCLUDED_
#define _STATE_H_INCLUDED_
#include "lib/containers/double-chain.h"
#include "lib/containers/map.h"
#include "lib/containers/vector.h"
#include "policer_flow.h"
#include "lib/nf_time.h"

struct State {
  struct Map* fm; 
  struct Vector* fk; // Keys
  struct Vector* fv; // Values
  struct DoubleChain* heap;
  int max_flows;
};

struct State* alloc_state(int max_flows);

#endif//_STATE_H_INCLUDED_
