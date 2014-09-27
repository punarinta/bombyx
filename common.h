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

#ifdef __linux__
    #include <mcheck.h>
#endif

#define MIN_VARIABLES       1000
#define MAX_VARIABLES       100000

#define MIN_BLOCKS       	1000
#define MAX_BLOCKS       	100000

#define ERR_NO_MEMORY       1
#define ERR_SYNTAX          2
#define ERR_TOO_LONG        3

var_table_t *vars;
block_table_t *blocks;
unsigned int vars_count;
unsigned int blocks_count;

unsigned int gl_error;
jmp_buf error_exit;
BYTE verbose;
BYTE gl_level;
unsigned int gl_block;
BYTE run_flag[256];     // this is only for statements, not for blocks
DWORD ret_point[256];
char *level_expr[256];

char *code;
unsigned int code_pos;
unsigned int code_length;
double started_at;

#endif
