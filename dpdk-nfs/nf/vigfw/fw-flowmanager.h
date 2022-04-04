#ifndef _FLOWMANAGER_H_INCLUDED_
#define _FLOWMANAGER_H_INCLUDED_

#include "fw-flow.h"

#include "lib/nf_time.h"

#include <stdbool.h>
#include <stdint.h>

struct FlowManager;

struct FlowManager* flow_manager_allocate(uint32_t expiration_time, uint64_t max_flows);

bool flow_manager_allocate_flow(struct FlowManager *manager, struct Flow* id, time_t time);
void flow_manager_expire(struct FlowManager *manager, time_t time);
bool flow_manager_get(struct FlowManager *manager, struct Flow *id, time_t time);

#endif //_FLOWMANAGER_H_INCLUDED_
