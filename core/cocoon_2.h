#ifndef _BOMBYX_COCOON_2_H_
#define _BOMBYX_COCOON_2_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cocoon_2.h"
#include "common.h"

cocoon_table_t *cocoon_table_create(int);
unsigned int cocoon_hash(cocoon_table_t *, char *);
cocoon_t *cocoon_lookup(cocoon_table_t *, char *);
cocoon_t *cocoon_add(bombyx_env_t *, cocoon_table_t *, char *);
void cocoon_table_delete(cocoon_table_t *);

#endif
