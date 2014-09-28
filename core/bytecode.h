#ifndef _BOMBYX_BYTECODE_H_
#define _BOMBYX_BYTECODE_H_ 1

#include "../common.h"
#include "var.h"

#define BC_INITIAL_SIZE     25000
#define BC_GROW_SIZE        10000

#define BCO_VAR             1
#define BCO_VARX            2
#define BCO_AS_STRING       3
#define BCO_AS_DOUBLE       4
#define BCO_AS_VAR          5
#define BCO_PRINT           6
#define BCO_MICROTIME       7
#define BCO_VAR_SET         8
#define BCO_ADD             10
#define BCO_SUB             11
#define BCO_MUL             12
#define BCO_DIV             13
#define BCO_INCR            14
#define BCO_DECR            15
#define BCO_SWAP            16
#define BCO_IF              20
#define BCO_ELSE            21
#define BCO_WHILE           22
#define BCO_BLOCK           23
#define BCO_RETURN          24
#define BCO_RETURNX         25
#define BCO_BLOCK_START     26
#define BCO_BLOCK_END       27
#define BCO_CEIT        28      // Compare and Execute If True

unsigned int bc_pos;
unsigned int bc_length;
char *bytecode;
var *bc_stack[256];
unsigned int bc_stack_pos;

void bc_init();
void bc_free();
void bc_add_cmd(BYTE);
void bc_add_token(char *);
void bc_add_double(double x);
void bc_add_string(char *);
void bc_grow();
void bc_ready();
void bc_poo();

#endif
