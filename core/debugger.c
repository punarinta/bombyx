#include "debugger.h"

void debug_env(bombyx_env_t *env)
{
    printf("\n[ Allocations ]\n");
    printf("Vars allocated: %d\n", env->vars->size);
    printf("Blocks allocated: %d\n", env->blocks->size);
    printf("Cocoons allocated: %d\n", env->cocoons->size);
    printf("Doubles allocated: %zu\n", env->pool_of_doubles->n_chunks);

    printf("\n[ Globals ]\n");
    printf("Global error = %u\n", env->gl_error);
    printf("Leaf directory = %s\n", env->dir_leaf);
    printf("Home directory = %s\n", env->dir_home);
    printf("Exit pointer = %p\n", env->error_exit);

    printf("\n[ Code ]\n");
    printf("Code position = %ld\n", env->code_pos);
    printf("Code length = %ld\n", env->code_length);

    printf("\n[ Bytecode ]\n");
    printf("Level = %u\n", env->gl_level);
    printf("Length = %u\n", env->bc_length);
    printf("Position = %u\n", env->bc_pos);
    printf("Steps = %u\n", env->bc_ops);

    printf("\n[ Stack ]\n");
    printf("Stack size = %u\n", env->bc_stack_size);

#ifdef BOMBYX_WEB
    printf("\n[ Web ]\n");
    printf("Thread ID = %d\n", env->thread_id);
#endif

    printf("\n");
}
