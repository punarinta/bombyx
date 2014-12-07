#ifndef _COCOON_WEB_H_
#define _COCOON_WEB_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common.h"
#include "../../core/map_2.h"
#include "../../core/var_2.h"
#include "../../vendor/jansson.h"

char *strtok_r(char *, const char *, char **);

#ifdef __cplusplus
extern "C" {
#endif

var version_();

#ifdef __cplusplus
}
#endif


#endif
