#ifndef _BOMBYX_MAP_H_
#define _BOMBYX_MAP_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

map_table_t *map_table_create(int);
unsigned int map_hash(map_table_t *, char *);
map_t *map_lookup(map_table_t *, char *);
map_t *map_add(map_table_t *, char *, var);
int map_delete(map_table_t *, char *);
void map_table_delete(map_table_t *);

#endif
