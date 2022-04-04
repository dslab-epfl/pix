#include "klee/klee.h"
#include "lib/containers/vector.h"
#include "vector-stub-control.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define PREALLOC_SIZE (256)
#define NUM_ELEMS (3)

struct Vector {
  char *id;
  char *data_type;
  uint8_t *data;
  int elem_size;
  int capacity;
  int elems_claimed;
  int index_claimed[NUM_ELEMS];
  int field_count;
  int nested_field_count;
  char *cell_type;

  vector_init_elem *init_elem;

  vector_entry_condition *ent_cond;
  void *ent_cond_state;
};

void vector_set_id(struct Vector *vec, char *id, char *data_type) {
  vec->id = malloc(strlen(id) + 1);
  strcpy(vec->id, id);
  vec->data_type = malloc(strlen(data_type) + 1);
  strcpy(vec->data_type, data_type);
}

int vector_allocate(int elem_size, unsigned capacity,
                    vector_init_elem *init_elem, struct Vector **vector_out) {
  klee_trace_ret();
  /* To differentiate between different vectors */
  klee_trace_param_ptr(vector_out, sizeof(struct Vector *), "vector_out");

  int allocation_succeeded = klee_int("vector_alloc_success");
  if (!allocation_succeeded)
    return 0;

  *vector_out = malloc(sizeof(struct Vector));
  klee_make_symbolic(*vector_out, sizeof(struct Vector), "vector");
  klee_possibly_havoc(*vector_out, sizeof(struct Vector), "vector");
  (*vector_out)->data = calloc(NUM_ELEMS, elem_size);
  klee_make_symbolic((*vector_out)->data, elem_size * NUM_ELEMS, "vector_data");
  // Do not call init elem, to preserve the elems being symbolic.
  // for (int n = 0; n < NUM_ELEMS; n++) {
  //  init_elem((*vector_out)->data + (elem_size*n));
  //}
  (*vector_out)->elem_size = elem_size;
  (*vector_out)->capacity = capacity;
  (*vector_out)->init_elem = init_elem;
  (*vector_out)->elems_claimed = 0;
  (*vector_out)->field_count = 0;
  (*vector_out)->nested_field_count = 0;
  (*vector_out)->ent_cond = NULL;
  (*vector_out)->ent_cond_state = NULL;
  klee_forbid_access((*vector_out)->data, elem_size * NUM_ELEMS,
                     "private state");
  return 1;
}

void vector_reset(struct Vector *vector) {
  // Do not trace. This function is an internal knob of the model.
  // TODO: reallocate vector->data to avoid having the same pointer?
  klee_allow_access(vector->data, vector->elem_size * NUM_ELEMS);
  klee_make_symbolic(vector->data, NUM_ELEMS * vector->elem_size,
                     "vector_data_reset");
  vector->elems_claimed = 0;
  // Do not call init elem, to preserve the elems being symbolic.
  // for (int n = 0; n < NUM_ELEMS; n++) {
  //  vector->init_elem(vector->data + (vector->elem_size*n));
  //}
  klee_forbid_access(vector->data, vector->elem_size * NUM_ELEMS,
                     "private state");
}

void vector_set_entry_condition(struct Vector *vector,
                                vector_entry_condition *cond, void *state) {
  vector->ent_cond = cond;
  vector->ent_cond_state = state;
}

void vector_borrow(struct Vector *vector, int index, void **val_out) {
  klee_trace_ret();
  // Avoid dumping the actual contents of vector.
  /* To differentiate between different vectors */
  klee_trace_param_u64((uint64_t)vector, "vector");

  klee_assert(vector->elems_claimed < NUM_ELEMS);
  void *cell = vector->data + vector->elems_claimed * vector->elem_size;

  klee_allow_access(vector->data, vector->elem_size * NUM_ELEMS);

  if (vector->ent_cond) {
    klee_assume(vector->ent_cond(cell, vector->ent_cond_state));
  }

  vector->index_claimed[vector->elems_claimed] = index;
  vector->elems_claimed += 1;
  *val_out = cell;
  ds_path_1();
}

void vector_return(struct Vector *vector, int index, void *value) {
  klee_trace_ret();
  // Avoid dumping the actual contents of vector
  /* To differentiate between different vectors */
  klee_trace_param_u64((uint64_t)vector, "vector");

  if (vector->ent_cond) {
    klee_assert(vector->ent_cond(value, vector->ent_cond_state));
  }

  int belongs = 0;
  for (int i = 0; i < vector->elems_claimed; ++i) {
    if (vector->data + i * vector->elem_size == value) {
      klee_assert(vector->index_claimed[i] == index);
      belongs = 1;
    }
  }
  klee_assert(belongs);
  klee_forbid_access(vector->data, vector->elem_size * NUM_ELEMS,
                     "private state");
  ds_path_1();
}