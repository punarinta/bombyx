#ifndef _BOMBYX_COMMON_H_
#define _BOMBYX_COMMON_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <setjmp.h>
#include <dlfcn.h>
#include "core/var.h"
#include "core/block.h"
#include "core/expression.h"

#define BOMBYX_DEBUG  1
#define BOMBYX_MCHECK 0

#if BOMBYX_MCHECK
    #include <mcheck.h>
#endif

#define MIN_VARIABLES       1000
#define MAX_VARIABLES       100000

#define MIN_BLOCKS       	1000
#define MAX_BLOCKS       	100000

#define BOMBYX_STACK_SIZE   256

var_table_t *vars;
block_table_t *blocks;
unsigned int vars_count;
unsigned int blocks_count;

unsigned int gl_error;
jmp_buf error_exit;
BYTE verbose;
BYTE gl_level;
BYTE run_flag[BOMBYX_STACK_SIZE];     // this is only for statements, not for blocks
DWORD ret_point[BOMBYX_STACK_SIZE];

char *code;
unsigned int code_pos;
unsigned int code_length;
double started_at;

#endif
