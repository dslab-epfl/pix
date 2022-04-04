#include "lib/containers/double-chain.h"
#include <assert.h>
#include <klee/klee.h>
#include <stdlib.h>
#include <string.h>

// TODO: double check that this model is enough for the NAT scenario

#define ALLOW(chain) klee_allow_access((chain), sizeof(struct DoubleChain))
#define DENY(chain)                                                            \
  klee_forbid_access((chain), sizeof(struct DoubleChain),                      \
                     "allocated_chain_do_not_dereference")

struct DoubleChain {
  char *id;
  char *data_type;
  int dchain_out_of_space;
  int new_index;
};

char *prefix; /* For tracing */

void dchain_set_id(struct DoubleChain *chain, char *id, char *data_type) {
  ALLOW(chain);
  chain->id = malloc(strlen(id) + 1);
  strcpy(chain->id, id);
  chain->data_type = malloc(strlen(data_type) + 1);
  strcpy(chain->data_type, data_type);
  DENY(chain);
}

__attribute__((noinline)) int dchain_allocate(int index_range,
                                              struct DoubleChain **chain_out) {
  klee_trace_ret();
  /* To differentiate between different chains */
  klee_trace_param_ptr_directed(chain_out, sizeof(struct DoubleChain *),
                                "chain_out", TD_OUT);

  // TODO not needed if malloc can fail
  int is_dchain_allocated = klee_int("is_dchain_allocated");
  *chain_out = malloc(sizeof(struct DoubleChain));

  klee_possibly_havoc(*chain_out, sizeof(struct DoubleChain), "chain_out");

  if (is_dchain_allocated & (*chain_out != NULL)) {
    memset(*chain_out, 0, sizeof(struct DoubleChain));
    (*chain_out)->new_index = klee_int("new_index"); 
    klee_assume(0 <= (*chain_out)->new_index);
    klee_assume((*chain_out)->new_index < index_range);
    (*chain_out)->dchain_out_of_space = klee_int("dchain_out_of_space");
    TRACE_VAL((uint32_t)(*chain_out), "dchain", _u32)
    DENY(*chain_out);
    return 1;
  } else {
    return 0;
  }
}

__attribute__((noinline)) int
dchain_allocate_new_index(struct DoubleChain *chain, int *index_out,
                          time_t time) {
  klee_trace_ret();
  /* To differentiate between different chains */
  klee_trace_param_u64((uint64_t)chain, "chain");

  TRACE_VAL((uint32_t)chain, "dchain", _u32)
  ALLOW(chain);
  TRACE_VAR(chain->dchain_out_of_space, "dchain_out_of_space")
  if (chain->dchain_out_of_space) {
    DENY(chain);
    ds_path_1();
    return 0;
  }

  *index_out = chain->new_index;
  DENY(chain);
  ds_path_2();
  return 1;
}

__attribute__((noinline)) int dchain_rejuvenate_index(struct DoubleChain *chain,
                                                      int index, time_t time) {
  klee_trace_ret();
  /* To differentiate between different chains */
  klee_trace_param_u64((uint64_t)chain, "chain");

  klee_assert(chain != NULL);
  ds_path_1();
  return 1;
}

__attribute__((noinline)) int dchain_expire_one_index(struct DoubleChain *chain,
                                                      int *index_out,
                                                      time_t time) {
  klee_trace_ret();
  /* To differentiate between different chains */
  klee_trace_param_u64((uint64_t)chain, "chain");

  klee_assert(chain != NULL);

  klee_assert(0 && "not supported in this model");
  ds_path_1();
  return 0;
}

__attribute__((noinline)) int
dchain_is_index_allocated(struct DoubleChain *chain, int index) {
  klee_trace_ret();
  /* To differentiate between different chains */
  klee_trace_param_u64((uint64_t)chain, "chain");

  klee_assert(chain != NULL);
  ds_path_1();
  ALLOW(chain);
  char *sym_name = "_in_";
  char *final_sym_name = (char *)malloc(1 + strlen(chain->data_type) +
                                        strlen(sym_name) + strlen(chain->id));
  strcpy(final_sym_name, chain->data_type);
  strcat(final_sym_name, sym_name);
  strcat(final_sym_name, chain->id);
  DENY(chain);
  return klee_int(final_sym_name);
}

void dchain_make_space(struct DoubleChain *chain, int nfreed) {
  // Do not trace. this function is internal for the Expirator model.
  ALLOW(chain);
  chain->dchain_out_of_space = 0; // Should be conditional on nfreed
  DENY(chain);
}

void dchain_reset(struct DoubleChain *chain, int index_range) {
  // Do not trace. This function is an internal knob of the model.
  ALLOW(chain);
  chain->new_index = klee_int("new_index"); // Ideally should use data type here too.
  klee_assume(0 <= chain->new_index);
  klee_assume(chain->new_index < index_range);

  char *sym_name = ".is_full";
  char *final_sym_name = (char *)malloc(1 + strlen(sym_name) + strlen(chain->id));
  strcpy(final_sym_name, chain->id);
  strcat(final_sym_name, sym_name);
  chain->dchain_out_of_space = klee_int(final_sym_name);
  DENY(chain);
}