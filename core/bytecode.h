#ifndef _BOMBYX_BYTECODE_H_
#define _BOMBYX_BYTECODE_H_ 1

#include "../common.h"
#include "var.h"

#define BC_INITIAL_SIZE         25000
#define BC_GROW_SIZE            10000
#define POOL_OF_DOUBLES_SIZE    1024

// TODO: group fully skippable operations to have the same bit

#define BCO_IDLE            0
#define BCO_VAR             1
#define BCO_VARX            2
#define BCO_AS_STRING       3
#define BCO_AS_DOUBLE       4
#define BCO_AS_VAR          5

#define BCO_SET             10
#define BCO_ADD             11
#define BCO_SUB             12
#define BCO_MUL             13
#define BCO_DIV             14
#define BCO_INCR            15
#define BCO_DECR            16
#define BCO_UNARY_MINUS     17
#define BCO_INVERT          18
#define BCO_CMP             19
#define BCO_CMP_NOT         20
#define BCO_AND             21
#define BCO_OR              22
#define BCO_LESS            23
#define BCO_LESS_EQ         24
#define BCO_MORE            25
#define BCO_MORE_EQ         26

#define BCO_IF              40
#define BCO_ELSE            41
#define BCO_WHILE           42
#define BCO_BLOCK_DEF       43
#define BCO_RETURN          44
#define BCO_CLEAR_STACK     45
#define BCO_BLOCK_START     46
#define BCO_BLOCK_END       47
#define BCO_CEIT            48      // Compare and Execute If True
#define BCO_CALL            49
#define BCO_SKIP            50

#define BCO_PRINT           200
#define BCO_MICROTIME       201
#define BCO_SWAP            202


unsigned int bc_ops;
unsigned int bc_pos;
unsigned int bc_length;
BYTE *bytecode;
var bc_stack[BOMBYX_STACK_SIZE];
unsigned int bc_stack_size;

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
