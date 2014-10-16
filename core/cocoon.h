#ifndef _BOMBYX_COCOON_H_
#define _BOMBYX_COCOON_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _cocoon_t_
{
    char *name;
    struct _cocoon_t_ *next;
    void *ptr;
} cocoon_t;

typedef struct _cocoon_table_t_
{
    unsigned int size;
    cocoon_t **table;
} cocoon_table_t;

cocoon_table_t *cocoon_table_create(int);
unsigned int cocoon_hash(cocoon_table_t *, char *);
cocoon_t *cocoon_lookup(cocoon_table_t *, char *);
cocoon_t *cocoon_add(cocoon_table_t *, char *);
void cocoon_table_delete(cocoon_table_t *);

#endif
