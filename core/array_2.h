#ifndef _BOMBYX_ARRAY_2_H_
#define _BOMBYX_ARRAY_2_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "array.h"
#include "var.h"
#include "common.h"

array_t *array_create(size_t);
void array_push(array_t *, var);
void array_set_elem(bombyx_env_t *, array_t *, unsigned int, var);
void array_delete(bombyx_env_t *, array_t *);
array_t *array_clone(bombyx_env_t *, array_t *);

#endif
