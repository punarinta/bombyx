#ifndef _BOMBYX_MAP_2_H_
#define _BOMBYX_MAP_2_H_ 1

#include "map.h"
#include "common.h"

map_table_t *map_table_create(int);
unsigned int map_hash(map_table_t *, char *);
map_t *map_lookup(map_table_t *, char *);
map_t *map_add(map_table_t *, char *, var);
int map_delete(map_table_t *, char *);
void map_table_delete(bombyx_env_t *env, map_table_t *);
map_table_t *map_table_clone(bombyx_env_t *, map_table_t *);

#endif
