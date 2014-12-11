#ifndef _BOMBYX_COMMON_H_
#define _BOMBYX_COMMON_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <setjmp.h>
#include <dlfcn.h>
#include "var.h"
#include "block.h"
#include "cocoon.h"
#include "challoc.h"
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

#define MIN_VARIABLES           1000
#define MAX_VARIABLES           100000

#define MIN_BLOCKS       	    1000
#define MAX_BLOCKS       	    100000

#define MIN_COCOONS       	    50
#define MAX_COCOONS       	    1000

#define BOMBYX_STACK_SIZE       256
#define POOL_OF_DOUBLES_SIZE    1024



#ifdef WEB_BUILD
    #define web_puts(a, b) FCGX_PutS(b, a->request.out)
    #define web_printf(a, b, ...) FCGX_FPrintF(a->request.out, b, __VA_ARGS__)
    #define WEB_NEWLINE "<br>"
#else
    #define web_puts(a, b) puts(b)
    #define web_printf(a, b, ...) printf(b, __VA_ARGS__)
    #define WEB_NEWLINE "\n"
#endif

typedef struct _web_data_
{
    char *http_content;
    size_t http_length;
    BYTE body_started;
} web_data;

typedef struct _bombyx_env_t_
{
    var_table_t *vars;
    block_table_t *blocks;
    cocoon_table_t *cocoons;
    ChunkAllocator* pool_of_doubles;

    unsigned int gl_error;
    char dir_leaf[256];
    char dir_home[256];
    jmp_buf error_exit;

    // levels
    BYTE gl_level;
    BYTE run_flag[BOMBYX_STACK_SIZE];       // this is only for statements, not for blocks
    DWORD ret_point[BOMBYX_STACK_SIZE];

    // bytecode
    unsigned int bc_ops;
    unsigned int bc_pos;
    unsigned int bc_length;
    BYTE *bytecode;
    var bc_stack[BOMBYX_STACK_SIZE];
    unsigned int bc_stack_size;

    char *code;
    size_t code_pos;
    size_t code_length;
    FCGX_Request request;
    int thread_id;
    web_data *wd;
} bombyx_env_t;

// thread safe variables
BYTE verbose;   // read-only

#endif
