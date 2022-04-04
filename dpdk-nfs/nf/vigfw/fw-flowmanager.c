#include "fw-flowmanager.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h> //for memcpy
#include <rte_ethdev.h>

#include "lib/containers/double-chain.h"
#include "lib/containers/map.h"
#include "lib/containers/vector.h"
#include "lib/expirator.h"

#include "fw-state.h"

struct FlowManager {
  struct State *state;
  uint32_t expiration_time; /*nanoseconds*/
};

struct FlowManager *flow_manager_allocate(uint32_t expiration_time,
                                          uint64_t max_flows) {
  struct FlowManager *manager =
      (struct FlowManager *)malloc(sizeof(struct FlowManager));
  if (manager == NULL) {
    return NULL;
  }
  manager->state = alloc_state(max_flows);
  if (manager->state == NULL) {
    return NULL;
  }
  manager->expiration_time = expiration_time;
  return manager;
}

bool flow_manager_allocate_flow(struct FlowManager *manager, struct Flow *id, time_t time) {
  int index;
  if (dchain_allocate_new_index(manager->state->heap, &index, time) == 0) {
    return false;
  }

  struct Flow *key = 0;
  vector_borrow(manager->state->fv, index, (void **)&key);
  memcpy((void *)key, (void *)id, sizeof(struct Flow));
  map_put(manager->state->fm, key, index);
  vector_return(manager->state->fv, index, key);
  return true;
}

void flow_manager_expire(struct FlowManager *manager, time_t time) {
  assert(time >= 0); // we don't support the past
  assert(sizeof(time_t) <= sizeof(uint64_t));
  uint64_t time_u = (uint64_t)time; // OK because of the two asserts
  time_t last_time =
      time_u - manager->expiration_time;
  expire_items_single_map(manager->state->heap, manager->state->fv,
                          manager->state->fm, last_time);
}

bool flow_manager_get(struct FlowManager *manager, struct Flow *id,
                               time_t time) {
  int index;
  if (map_get(manager->state->fm, id, &index) == 0) {
    return false;
  }
  dchain_rejuvenate_index(manager->state->heap, index, time);
  return true;
}