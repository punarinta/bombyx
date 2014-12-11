#ifndef _BOMBYX_BYTECODE_H_
#define _BOMBYX_BYTECODE_H_ 1

#define BC_INITIAL_SIZE         25000
#define BC_GROW_SIZE            10000

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
#define BCO_ACCESS          13
#define BCO_FROM_JSON       14
#define BCO_XCALL           15

#define BCO_SET             32
#define BCO_SET_ELEM        33
#define BCO_ADD_ELEM        34
#define BCO_ADD             35
#define BCO_SUB             36
#define BCO_MUL             37
#define BCO_DIV             38
#define BCO_INCR            39
#define BCO_DECR            40
#define BCO_UNARY_MINUS     41
#define BCO_INVERT          42
#define BCO_CMP             43
#define BCO_CMP_NOT         44
#define BCO_AND             45
#define BCO_OR              46
#define BCO_LESS            47
#define BCO_LESS_EQ         48
#define BCO_MORE            49
#define BCO_MORE_EQ         50

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

#include "common.h"

void bc_init(bombyx_env_t *);
void bc_free(bombyx_env_t *);
void bc_add_cmd(bombyx_env_t *, BYTE);
void bc_add_token(bombyx_env_t *, char *);
void bc_add_double(bombyx_env_t *, double);
void bc_add_string(bombyx_env_t *, char *);
void bc_grow(bombyx_env_t *);
void bc_ready(bombyx_env_t *);
void bc_poo(bombyx_env_t *);

#endif
