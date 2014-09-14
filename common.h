#ifndef _COMMON_H_
#define _COMMON_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "core/var.h"
#include "core/expression.h"

#define MIN_VARIABLES       100
#define MAX_VARIABLES       100000

#define ERR_NO_MEMORY       1
#define ERR_SYNTAX          2
#define ERR_TOO_LONG        3

var *vars;
unsigned int vars_count;

char *code;
unsigned int code_pos;
unsigned int code_length;
char gl_errmsg[256];

int larva_stop(int);

#endif
