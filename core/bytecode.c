#include "bytecode.h"

void bc_init()
{
    bc_pos = 0;
    bc_length = BC_INITIAL_SIZE;
    bytecode = malloc(BC_INITIAL_SIZE);
}

void bc_free()
{
    if (bytecode) free(bytecode);
    bytecode = NULL;
}

void bc_add_cmd(BYTE cmd)
{
    if (bc_pos >= bc_length) bc_grow();

    bytecode[bc_pos++] = cmd;
}

void bc_add_token(char *token)
{
    unsigned int size = strlen(token);

    if (bc_pos >= bc_length - size - 1) bc_grow();

    bytecode[bc_pos++] = size;
    memcpy(bytecode + bc_pos, token, size);
}

void bc_add_double(double x)
{
    unsigned int size = sizeof(double);

    if (bc_pos >= bc_length - size) bc_grow();

    memcpy(bytecode + bc_pos, &x, size);
}

void bc_add_string(char *str)
{
    unsigned int size = strlen(str);

    if (bc_pos >= bc_length - size - 2) bc_grow();

    bytecode[bc_pos++] = size;
    bytecode[bc_pos++] = size % 256;
    memcpy(bytecode + bc_pos, str, size);
}

void bc_grow()
{
    bc_length += BC_GROW_SIZE;
    bytecode = (char *) realloc(bytecode, bc_length);
}

void bc_poo()
{
    for (unsigned int i = 0; i < bc_pos; i++)
    {
        fprintf(stdout, "%04X: %c\n", i, bytecode[i]);
    }
}