#include "bytecode.h"
#include "challoc.h"

void bc_init(bombyx_env_t *env)
{
    env->bc_pos = 0;
    env->bc_length = BC_INITIAL_SIZE;
    env->bytecode = malloc(BC_INITIAL_SIZE);
}

void bc_free(bombyx_env_t *env)
{
    if (env->bytecode) free(env->bytecode);
    env->bytecode = NULL;
}

void bc_add_cmd(bombyx_env_t *env, BYTE cmd)
{
    if (env->bc_pos >= env->bc_length) bc_grow(env);

    env->bytecode[env->bc_pos++] = cmd;
}

void bc_add_token(bombyx_env_t *env, char *token)
{
    unsigned int size = strlen(token);

    if (env->bc_pos >= env->bc_length - size - 1) bc_grow(env);

    env->bytecode[env->bc_pos++] = size;
    memcpy(env->bytecode + env->bc_pos, token, size);

    env->bc_pos += size;
}

void bc_add_double(bombyx_env_t *env, double x)
{
    if (env->bc_pos >= env->bc_length - sizeof(double)) bc_grow(env);

    *(double *)(env->bytecode + env->bc_pos) = x;

    env->bc_pos += sizeof(double);
}

void bc_add_string(bombyx_env_t *env, char *str)
{
    uint16 size = strlen(str);
    if (env->bc_pos >= env->bc_length - size - sizeof(uint16)) bc_grow(env);

    memcpy(env->bytecode + env->bc_pos, &size, sizeof(uint16));
    env->bc_pos += sizeof(uint16);

    memcpy(env->bytecode + env->bc_pos, str, size);
    env->bc_pos += size;
}

void bc_grow(bombyx_env_t *env)
{
    env->bc_length += BC_GROW_SIZE;
    env->bytecode = (unsigned char *) realloc(env->bytecode, env->bc_length);
}

void bc_ready(bombyx_env_t *env)
{
    env->bc_length = env->bc_pos + 1;
    env->bytecode = (unsigned char *) realloc(env->bytecode, env->bc_length);
    env->bytecode[env->bc_pos] = 0;
    env->bc_pos = 0;
    env->bc_ops = 0;

    // prepare stack
    env->bc_stack_size = 0;

    for (unsigned int i = 0; i < BOMBYX_STACK_SIZE; i++)
    {
        env->bc_stack[i].type = 0;
        env->bc_stack[i].data_size = 0;
        env->bc_stack[i].name = NULL;
        env->bc_stack[i].data = NULL;
    }
}

void bc_poo(bombyx_env_t *env)
{
    for (unsigned int i = 0; i < env->bc_length; i++)
    {
        fprintf(stdout, "%04d: ", i);
        if (isalpha(env->bytecode[i]) || isdigit(env->bytecode[i]) || env->bytecode[i]=='_') fprintf(stdout, "%c\n", env->bytecode[i]);
        else fprintf(stdout, "[%03d]\n", env->bytecode[i]);
    }
}
