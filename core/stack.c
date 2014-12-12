#include "common.h"
#include "larva.h"
#include "var_2.h"

inline void stack_push(bombyx_env_t *env, var v)
{
    if (env->bc_stack_size > BOMBYX_STACK_SIZE - 1)
    {
        larva_error(env, "Stack overflow. :(");
    }
    v.level = env->gl_level;
    env->bc_stack[env->bc_stack_size++] = v;
}

inline var stack_pop(bombyx_env_t *env)
{
    if (env->bc_stack_size <= 0)
    {
        larva_error(env, "Stack is empty. Pop failed.");
    }
    return env->bc_stack[--env->bc_stack_size];
}

/*
    Must be called before the level is left
*/
void stack_clear(bombyx_env_t *env)
{
    while (env->bc_stack_size)
    {
        if (env->bc_stack[env->bc_stack_size - 1].level < env->gl_level) return;
        var_unset(env, &env->bc_stack[--env->bc_stack_size]);
    }
}

void stack_poo(bombyx_env_t *env)
{
    puts("================ STACK ===============");
    printf("Stack size = %u.\n", env->bc_stack_size);
    for (int i = env->bc_stack_size - 1; i >= 0; i--)
    {
        fprintf(stdout, "%04d: ", i);
        var_echo(env, &env->bc_stack[i]);
        fprintf(stdout, "\n");
    }
    puts("======================================");
}