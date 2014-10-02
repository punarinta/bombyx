#include "bytecode.h"
#include "challoc.h"

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

    chdestroy(&pool_of_doubles);
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

    bc_pos += size;
}

void bc_add_double(double x)
{
    if (bc_pos >= bc_length - sizeof(double)) bc_grow();

    *(double *)(bytecode + bc_pos) = x;

    bc_pos += sizeof(double);
}

void bc_add_string(char *str)
{
    unsigned int size = strlen(str);

    if (bc_pos >= bc_length - size - 2) bc_grow();

    bytecode[bc_pos++] = size % 256;
    bytecode[bc_pos++] = size / 256;
    memcpy(bytecode + bc_pos, str, size);

    bc_pos += size;
}

void bc_grow()
{
    bc_length += BC_GROW_SIZE;
    bytecode = (unsigned char *) realloc(bytecode, bc_length);
}

void bc_ready()
{
    bc_length = bc_pos + 1;
    bytecode = (unsigned char *) realloc(bytecode, bc_length);
    bytecode[bc_pos] = 0;
    bc_pos = 0;
    bc_ops = 0;

    // prepare stack
    bc_stack_size = 0;
    for (unsigned int i = 0; i < BOMBYX_STACK_SIZE; i++)
    {
        bc_stack[i].type = 0;
        bc_stack[i].data_size = 0;
        bc_stack[i].name = NULL;
        bc_stack[i].data = NULL;
    }

    pool_of_doubles = chcreate(POOL_OF_DOUBLES_SIZE, sizeof(double));
}

void bc_poo()
{
    for (unsigned int i = 0; i < bc_length; i++)
    {
        fprintf(stdout, "%04d: ", i);
        if (isalpha(bytecode[i]) || isdigit(bytecode[i]) || bytecode[i]=='_') fprintf(stdout, "%c\n", bytecode[i]);
        else fprintf(stdout, "[%03d]\n", bytecode[i]);
    }
}