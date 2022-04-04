#ifndef _VECTOR_STUB_CONTROL_H_INCLUDED_
#define _VECTOR_STUB_CONTROL_H_INCLUDED_

#include "lib/containers/vector.h"
#include "str-descr.h"

#include <stdbool.h>

typedef bool vector_entry_condition(void *value, void *state);

void vector_set_entry_condition(struct Vector *vector,
                                vector_entry_condition *cond, void *state);

void vector_reset(struct Vector *vector);

void vector_set_id(struct Vector *vec, char *id, char* data_type);

#endif // _VECTOR_STUB_CONTROL_H_INCLUDED_
