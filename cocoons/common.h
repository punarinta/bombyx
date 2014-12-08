#ifndef _COCOON_COMMON_H_
#define _COCOON_COMMON_H_

#include "../core/common.h"
#include "../core/var.h"
#include "../core/sys.h"
#include "../core/map.h"
#include "../core/array.h"
#include "fcgiapp.h"

void larva_error(bombyx_env_t *env, char *, ...);
var cocoon_error(bombyx_env_t *env, char *, ...);
void random_string(char *, const int);

#endif