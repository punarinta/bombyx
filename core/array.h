#ifndef _BOMBYX_ARRAY_H_
#define _BOMBYX_ARRAY_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "var.h"

typedef struct _array_t_
{
    char *name;
    struct _array_t_ *next;
    var v;
} array_t;

typedef struct _array_table_t_
{
    unsigned int size;
    array_t **table;
} array_table_t;

array_table_t *array_table_create(int);
unsigned int array_hash(array_table_t *, char *);
array_t *array_lookup(array_table_t *, char *);
array_t *array_add(array_table_t *, char *);
int array_delete(array_table_t *, char *);
void array_table_delete(array_table_t *);

#endif
