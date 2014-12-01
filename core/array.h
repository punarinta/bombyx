#ifndef _BOMBYX_ARRAY_H_
#define _BOMBYX_ARRAY_H_ 1

#include "var.h"

typedef struct _array_t_
{
    size_t size;
    size_t max_size;
    var **vars;
} array_t;

#endif
