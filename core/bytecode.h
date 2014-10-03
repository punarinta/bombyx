#ifndef _BOMBYX_BYTECODE_H_
#define _BOMBYX_BYTECODE_H_ 1

#include "../common.h"
#include "var.h"
#include "challoc.h"

#define BC_INITIAL_SIZE         25000
#define BC_GROW_SIZE            10000
#define POOL_OF_DOUBLES_SIZE    1024


#define IS_BCO_SKIPPABLE    224 // 0b11100000

#define BCO_IDLE            0

// unskippable go first

#define BCO_VAR             1
#define BCO_VARX            2
#define BCO_AS_STRING       3
#define BCO_AS_DOUBLE       4
#define BCO_AS_VAR          5
#define BCO_CALL            6
#define BCO_BLOCK_DEF       7
#define BCO_BLOCK_START     8
#define BCO_BLOCK_END       9
#define BCO_REVERSE_STACK   10
#define BCO_PARAM           11
#define BCO_PARAMX          12

#define BCO_SET             32
#define BCO_ADD             33
#define BCO_SUB             34
#define BCO_MUL             35
#define BCO_DIV             36
#define BCO_INCR            37
#define BCO_DECR            38
#define BCO_UNARY_MINUS     39
#define BCO_INVERT          40
#define BCO_CMP             41
#define BCO_CMP_NOT         42
#define BCO_AND             43
#define BCO_OR              44
#define BCO_LESS            45
#define BCO_LESS_EQ         46
#define BCO_MORE            47
#define BCO_MORE_EQ         48

#define BCO_IF              60
#define BCO_ELSE            61
#define BCO_WHILE           62
#define BCO_RETURN          63
#define BCO_CLEAR_STACK     64
#define BCO_CEIT            65      // Compare and Execute If True
#define BCO_SKIP            66

#define BCO_PRINT           200
#define BCO_MICROTIME       201
#define BCO_SWAP            202
#define BCO_AS_VOID         203


unsigned int bc_ops;
unsigned int bc_pos;
unsigned int bc_length;
BYTE *bytecode;
var bc_stack[BOMBYX_STACK_SIZE];
unsigned int bc_stack_size;

ChunkAllocator* pool_of_doubles;

void bc_init();
void bc_free();
void bc_add_cmd(BYTE);
void bc_add_token(char *);
void bc_add_double(double);
void bc_add_string(char *);
void bc_grow();
void bc_ready();
void bc_poo();

#endif
