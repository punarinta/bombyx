#ifndef _COCOON_COMMON_H_
#define _COCOON_COMMON_H_

#include "../core/var.h"
#include "../core/sys.h"
#include "../core/array.h"
#include "fcgiapp.h"

void larva_error(char *);
var cocoon_error(FCGX_Request *, char *);

#endif