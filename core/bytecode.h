#ifndef _BOMBYX_BYTECODE_H_
#define _BOMBYX_BYTECODE_H_ 1

#include "../common.h"

#define BC_INITIAL_SIZE     25000
#define BC_GROW_SIZE        10000

#define BCO_VAR         1
#define BCO_VARV        2

unsigned int bc_pos;
unsigned int bc_length;
char *bytecode;

void bc_init();
void bc_free();
void bc_add_cmd(BYTE);
void bc_add_token(char *);
void bc_add_double(double x);
void bc_add_string(char *);
void bc_grow();
void bc_poo();

#endif
