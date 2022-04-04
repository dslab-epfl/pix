#include "double-chain.h"
#include <stdlib.h>
#include <string.h>

#ifdef DUMP_PERF_VARS
#include "lib/nf_log.h"
#endif

#define DCHAIN_RESERVED 1

struct dchain_cell {
  int prev;
  int next;
  time_t timestamp;
  int busy;
};

struct DoubleChain {
  struct dchain_cell *cells;
  int free_index;
  int capacity;
};

/** Helper functions ***/
int dchain_get_free_index(struct DoubleChain *chain) {
  int res;
  int start = chain->free_index;
  int index_range = chain->capacity;

#ifdef DUMP_PERF_VARS
  int traversals = 0;
#endif
  for (int i = start; i < start + index_range; i++) {
    res = i & (index_range - 1);

#ifdef DUMP_PERF_VARS
    traversals++;
#endif
    if (chain->cells[i].busy == 0) {
#ifdef DUMP_PERF_VARS
      NF_PERF_DEBUG("Dchain_get_free_index:Success:Num_port_traversals:%d",
                    traversals);
#endif
      return i;
    }
  }
  return -1;
}

int dchain_allocate(int index_range, struct DoubleChain **chain_out) {

  struct DoubleChain *old_chain_out = *chain_out;
  struct DoubleChain *chain_alloc = malloc(sizeof(struct DoubleChain));
  if (chain_alloc == NULL)
    return 0;
  *chain_out = (struct DoubleChain *)chain_alloc;

  struct dchain_cell *cells_alloc =
      malloc(sizeof(*cells_alloc) * (index_range + DCHAIN_RESERVED));
  if (cells_alloc == NULL) {
    free(chain_alloc);
    *chain_out = old_chain_out;
    return 0;
  }
  (*chain_out)->cells = cells_alloc;

  for (int i = 0; i < index_range; i++) {
    (*chain_out)->cells[i].timestamp = -1;
    (*chain_out)->cells[i].prev = -1;
    (*chain_out)->cells[i].next = -1;
    (*chain_out)->cells[i].busy = 0;
  }

  (*chain_out)->cells[index_range].timestamp = -1;
  (*chain_out)->cells[index_range].prev = -1;
  (*chain_out)->cells[index_range].next = -1;
  (*chain_out)->cells[index_range].busy = 1;

  (*chain_out)->free_index = 0;
  (*chain_out)->capacity = index_range;

  return 1;
}

int dchain_allocate_new_index(struct DoubleChain *chain, int *index_out,
                              time_usec_t time) {
  int index = dchain_get_free_index(chain);
  if (index == -1) {
    return 0; // No free index to allocate
  } else {
    int prev_first = chain->cells[chain->capacity].next;
    if (prev_first != -1) {
      chain->cells[prev_first].prev = index;
      chain->cells[index].next = prev_first;

    } else {
      chain->cells[chain->capacity].prev = index;
      chain->cells[index].next = chain->capacity;
    }

    chain->cells[chain->capacity].next = index; // This is the head of the chain
    chain->cells[index].prev = chain->capacity;
    chain->cells[index].timestamp = time;
    chain->cells[index].busy = 1;
    chain->free_index = (index + 1) % chain->capacity;
    *index_out = index;
    return 1;
  }
}

int dchain_rejuvenate_index(struct DoubleChain *chain, int index,
                            time_usec_t time) {
  if (chain->cells[index].busy == 0) {
    return 0;
  } else {

    int prev_prev = chain->cells[index].prev;
    int prev_next = chain->cells[index].next;
    chain->cells[index].timestamp = time;
    int prev_first = chain->cells[chain->capacity].next;
    if (prev_first != index) // Not already at the head of the queue
    {
      int prev_first = chain->cells[chain->capacity].next;
      chain->cells[chain->capacity].next =
          index; // This is the head of the chain
      chain->cells[index].prev = chain->capacity;
      chain->cells[index].next = prev_first;
      chain->cells[prev_prev].next = prev_next;
      chain->cells[prev_next].prev = prev_prev;
      chain->cells[prev_first].prev = index;
    }
    return 1;
  }
}

int dchain_expire_one_index(struct DoubleChain *chain, int *index_out,
                            time_usec_t time) {
  int index = chain->cells[chain->capacity].prev; // Oldest
  if (index == -1) {
    return 0; // Empty list
  } else {
    if (chain->cells[index].timestamp > time) {
      return 0; // Oldest is still fresh
    } else {
      int prev_next = chain->capacity;
      int prev_prev = chain->cells[index].prev;
      if (prev_prev != prev_next) // Not only item in queue
      {
        chain->cells[prev_prev].next = prev_next;
        chain->cells[prev_next].prev = prev_prev;

      } else {
        chain->cells[chain->capacity].prev = -1;
        chain->cells[chain->capacity].next = -1;
      }
      *index_out = index;
      chain->cells[index].busy = 0;
      return 1;
    }
  }
}
