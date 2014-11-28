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
#include "core/challoc.h"
#include "core/expression.h"
#include "fcgiapp.h"

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


#ifdef WEB_BUILD
    #define web_puts(a) FCGX_PutS(a, pRequest->out)
    #define web_printf(a, ...) FCGX_FPrintF(pRequest->out, a, __VA_ARGS__)
    #define WEB_NEWLINE "<br>"
#else
    #define web_puts puts
    #define web_printf printf
    #define WEB_NEWLINE "\n"
#endif


// thread safe variables
BYTE verbose;   // read-only


// thread unsafe, need to be moved away
var_table_t *vars;
block_table_t *blocks;
cocoon_table_t *cocoons;
ChunkAllocator* pool_of_doubles;

unsigned int gl_error;
char temp_error[256];
char dir_leaf[256];
char dir_home[256];
jmp_buf error_exit;
BYTE gl_level;
BYTE run_flag[BOMBYX_STACK_SIZE];       // this is only for statements, not for blocks
DWORD ret_point[BOMBYX_STACK_SIZE];

unsigned int bc_ops;
unsigned int bc_pos;
unsigned int bc_length;
BYTE *bytecode;
var bc_stack[BOMBYX_STACK_SIZE];
unsigned int bc_stack_size;

char *code;
size_t code_pos;
size_t code_length;
double started_at;
FCGX_Request *pRequest;

#endif
