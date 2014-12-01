#ifndef _BOMBYX_MAP_H_
#define _BOMBYX_MAP_H_ 1

#include "var.h"

typedef struct _map_t_
{
    char *name;
    var v;
    struct _map_t_ *next;
} map_t;

typedef struct _map_table_t_
{
    unsigned int size;
    map_t **table;
} map_table_t;

#endif
