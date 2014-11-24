#ifndef _BOMBYX_ARRAY_H_
#define _BOMBYX_ARRAY_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "var.h"

typedef struct _array_t_
{
    size_t size;
    var **vars;
} array_t;

array_t *array_create(size_t);
void array_delete(array_t *);
array_t *array_clone(array_t *);

#endif
