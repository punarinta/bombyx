#ifndef _BOMBYX_COMMON_H_
#define _BOMBYX_COMMON_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <setjmp.h>
#include <dlfcn.h>
#include "core/var.h"
#include "core/block.h"
#include "core/cocoon.h"
#include "core/expression.h"

// #define BOMBYX_DEBUG
// #define BOMBYX_MCHECK

#ifdef BOMBYX_DEBUG
    #define debug_verbose_puts(s) if (verbose) puts(s);
#else
    #define debug_verbose_puts(s)
#endif

#ifdef BOMBYX_MCHECK
    #include <mcheck.h>
#endif

#define MIN_VARIABLES       1000
#define MAX_VARIABLES       100000

#define MIN_BLOCKS       	1000
#define MAX_BLOCKS       	100000

#define MIN_COCOONS       	50
#define MAX_COCOONS       	1000

#define BOMBYX_STACK_SIZE   256

var_table_t *vars;
block_table_t *blocks;
cocoon_table_t *cocoons;
unsigned int vars_count;
unsigned int blocks_count;

unsigned int gl_error;
char temp_error[256];
jmp_buf error_exit;
BYTE verbose;
BYTE gl_level;
BYTE run_flag[BOMBYX_STACK_SIZE];       // this is only for statements, not for blocks
DWORD ret_point[BOMBYX_STACK_SIZE];

char *code;
size_t code_pos;
size_t code_length;
double started_at;

#endif
