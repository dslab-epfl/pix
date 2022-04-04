#ifndef _STATE_H_INCLUDED_
#define _STATE_H_INCLUDED_
#include "lib/containers/map.h"
#include "lib/containers/vector.h"
#include "fw-flow.h"
#include "lib/nf_time.h"

struct State {
  struct Map* fm;
  struct Vector* fv;
  struct DoubleChain* heap;
  int max_flows;
};

struct State* alloc_state(int max_flows);
#endif//_STATE_H_INCLUDED_
