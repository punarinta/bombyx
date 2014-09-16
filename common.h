#ifndef _COMMON_H_
#define _COMMON_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <setjmp.h>
#include "core/var.h"
#include "core/expression.h"

#define MIN_VARIABLES       100
#define MAX_VARIABLES       100000

#define ERR_NO_MEMORY       1
#define ERR_SYNTAX          2
#define ERR_TOO_LONG        3

var *vars;
unsigned int vars_count;
unsigned int gl_error;
jmp_buf error_exit;
BYTE verbose;

char *code;
unsigned int code_pos;
unsigned int code_length;
double started_at;

int larva_stop(int);
void larva_error();
double get_microtime();

#endif
