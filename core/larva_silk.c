#include "common.h"
#include "larva.h"
#include "var.h"
#include "var_2.h"
#include "map_2.h"
#include "array_2.h"
#include "cocoon_2.h"
#include "expression.h"
#include "map.h"
#include "array.h"
#include "block.h"
#include "sys.h"
#include "bytecode.h"
#include "../vendor/jansson.h"

inline void stack_push(bombyx_env_t *env, var v)
{
    v.level = env->gl_level;
    env->bc_stack[env->bc_stack_size++] = v;
}

inline var stack_pop(bombyx_env_t *env)
{
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
    for (int i = env->bc_stack_size - 1; i >= 0; i--)
    {
        fprintf(stdout, "%04d: ", i);
        var_echo(env, &env->bc_stack[i]);
        fprintf(stdout, "\n");
    }
    puts("======================================");
}

// execute byte-code
void larva_silk(bombyx_env_t *env)
{
    double d;
    var_t *vt;
    var v1, v2;
    env->gl_level = 0;
    BYTE level = 0;
    env->run_flag[0] = 0;    // do we need this?
    size_t size = 0;
    BYTE skip_mode = 0;
    int param_count = 0;
    block_t *parent_block = NULL;
    char token[PARSER_MAX_TOKEN_SIZE];
    char token2[PARSER_MAX_TOKEN_SIZE];

    bc_ready(env);

/*    puts("=============== BYTECODE =============");
    bc_poo(env);
    puts("======================================");*/
//exit(0);

    // reentry must be here, not above! => rewrite silk function later

    while (env->bc_pos < env->bc_length)
    {
        /*if (env->bytecode[bc_pos] == BCO_IDLE)
        {
            ++bc_pos;
            continue;
        }*/

        if (skip_mode)
        {
            if (env->bytecode[env->bc_pos] & IS_BCO_SKIPPABLE)
            {
                // bytecode >=32 --> can be skipped right now
                ++env->bc_pos;
                continue;
            }
        }
#ifdef BOMBYX_DEBUG
        else
        {
            ++env->bc_ops;
        }
#endif

        switch (env->bytecode[env->bc_pos++])
        {
            case BCO_AS_VAR:
            size = env->bytecode[env->bc_pos++];
            if (skip_mode)
            {
                env->bc_pos += size;
                break;
            }
            debug_verbose_puts("BCO_AS_VAR");
            memcpy(token, env->bytecode + env->bc_pos, size);
            token[size] = 0;
            vt = var_lookup_2(env->vars, token, size);
            if (!vt)
            {
                larva_error(env, "Unknown variable '%s'.", token);
            }
            env->bc_stack[env->bc_stack_size++] = var_as_var_t(env, vt);
            env->bc_pos += size;
            break;

            case BCO_SET:
            debug_verbose_puts("BCO_SET");
            v2 = stack_pop(env);

            if (!(env->bc_stack[env->bc_stack_size - 1].ref))
            {
            	larva_error(env, "Left part of an equation should be a variable.");
            }

            // note: v1 stays inside the stack

            // copy directly as 'v2' is not needed anymore
            op_assign(&vt->v, &v2);
            break;

            case BCO_SET_ELEM:
            size = env->bytecode[env->bc_pos++];
            if (skip_mode)
            {
                env->bc_pos += size;
                break;
            }

            debug_verbose_puts("BCO_SET_ELEM");
            v2 = stack_pop(env); // value
            v1 = stack_pop(env); // index

            // get vt name
            memcpy(token, env->bytecode + env->bc_pos, size);
            token[size] = 0;
            env->bc_pos += size;

            if (!(vt = var_lookup_2(env->vars, token, size)))
            {
                larva_error(env, "Variable '%s' does not exist.", token);
            }

            if (v1.type == VAR_STRING)
            {
                var *pv = var_apath(env, &vt->v, v1.data);

                if (pv) *pv = v2;
                else
                {
                    map_add(env, vt->v.data, v1.data, v2);
                }
            }
            else if (v1.type == VAR_DOUBLE)
            {
                *((array_t *)(vt->v.data))->vars[ (unsigned int)*(double *)v1.data ] = v2;
            }
            else
            {
                fprintf(stderr, "Object '%s' is not accessible with [] operator.", token);
                larva_error(env, 0);
            }

            // push the value to stack, note that 'v1' and 'v2' cannot be unset here
            memset(&v1, 0, sizeof(var));
            op_copy(env, &v1, &v2);
            stack_push(env, v1);
            break;

            case BCO_ADD_ELEM:
            debug_verbose_puts("BCO_ADD_ELEM");
            v1 = stack_pop(env); // value

            // get vt name
            size = env->bytecode[env->bc_pos++];
            memcpy(token, env->bytecode + env->bc_pos, size);
            token[size] = 0;
            env->bc_pos += size;

            if (!(vt = var_lookup_2(env->vars, token, size)))
            {
                larva_error(env, "Variable '%s' does not exist.", token);
            }

            if (vt->v.type == VAR_ARRAY)
            {
                array_push(vt->v.data, v1);
            }
            else
            {
                fprintf(stderr, "Pushing with [] operator works only for arrays.");
                larva_error(env, 0);
            }

            // push the value to stack, note that 'v1' cannot be unset here
            memset(&v2, 0, sizeof(var));
            op_copy(env, &v2, &v1);
            stack_push(env, v2);
            break;

            case BCO_AS_DOUBLE:
            if (skip_mode)
            {
                env->bc_pos += sizeof(double);
                break;
            }
            debug_verbose_puts("BCO_AS_DOUBLE");
            d = *(double *)(env->bytecode + env->bc_pos);
            env->bc_stack[env->bc_stack_size++] = var_as_double(env, d);
            env->bc_pos += sizeof(double);
            break;

            case BCO_AS_STRING:
            size = env->bytecode[env->bc_pos] + (env->bytecode[env->bc_pos + 1] << 8);
            env->bc_pos += 2;
            if (skip_mode)
            {
                env->bc_pos += size;
                break;
            }
            debug_verbose_puts("BCO_AS_STRING");
            memcpy(token, env->bytecode + env->bc_pos, size);
            token[size] = 0;
            stack_push(env, var_as_string(token, size));
            env->bc_pos += size;
            break;

            case BCO_FROM_JSON:
            memcpy(&size, env->bytecode + env->bc_pos, sizeof(uint16));
            env->bc_pos += sizeof(uint16);
            if (skip_mode)
            {
                env->bc_pos += size;
                break;
            }
            debug_verbose_puts("BCO_FROM_JSON");
            char *str = malloc(size + 1);
            memcpy(str, env->bytecode + env->bc_pos, size);
            str[size] = 0;
            env->bc_pos += size;
            stack_push(env, var_from_json(env, str));
            free(str);
            break;

            case BCO_ACCESS:
            size = env->bytecode[env->bc_pos++];
            if (skip_mode)
            {
                env->bc_pos += size;
                break;
            }
            debug_verbose_puts("BCO_ACCESS");
            memcpy(token, env->bytecode + env->bc_pos, size);
            token[size] = 0;
            env->bc_pos += size;

            v1 = stack_pop(env);

            if (!(vt = var_lookup_2(env->vars, token, size)))
            {
                larva_error(env, "Variable '%s' does not exist.", token);
            }

            if (v1.type == VAR_STRING)
            {
                var *pv = var_apath(env, &vt->v, v1.data);

                if (pv)
                {
                    op_copy(env, &v2, pv);
                }
                else
                {
                    memset(&v2, 0, sizeof(var));
                }

                stack_push(env, v2);
            }
            else if (v1.type == VAR_DOUBLE)
            {
                v2.data = NULL;
                var *pv = ((array_t *)(vt->v.data))->vars[ (unsigned int)*(double *)v1.data ];
                if (pv)
                {
                    op_copy(env, &v2, pv);
                    stack_push(env, v2);
                }
                else
                {
                    // element doesn't exist
                    // TODO: VERY DOUBTFUL
                    memset(&env->bc_stack[env->bc_stack_size++], 0, sizeof(var));
                }
            }
            else
            {
                larva_error(env, "Object '%s' is not accessible with [] operator.", token);
            }
            break;

            case BCO_CALL:
            debug_verbose_puts("BCO_CALL");
            size = env->bytecode[env->bc_pos++];
            if (skip_mode)
            {
                env->bc_pos += size;
                break;
            }
            memcpy(token, env->bytecode + env->bc_pos, size);
            token[size] = 0;
            env->bc_pos += size;
            block_t *this_block = block_lookup(env->blocks, token);
            if (!this_block)
            {
                larva_error(env, "Unknown function '%s'.", token);
            }
            else
            {
                ++env->gl_level;
                env->ret_point[env->gl_level] = env->bc_pos;
            	// step into
            	env->run_flag[env->gl_level] = RUN_BLOCK;
            	env->bc_pos = this_block->pos;
            }
            break;

            case BCO_XCALL:
            debug_verbose_puts("BCO_XCALL");
            size = env->bytecode[env->bc_pos++];
            if (skip_mode)
            {
                env->bc_pos += size;
                size = env->bytecode[env->bc_pos++];
                env->bc_pos += size;
                ++env->bc_pos;           // argc
                break;
            }
            memcpy(token, env->bytecode + env->bc_pos, size);
            token[size] = 0;
            env->bc_pos += size;
            size = env->bytecode[env->bc_pos++];
            memcpy(token2, env->bytecode + env->bc_pos, size);
            token2[size] = 0;
            env->bc_pos += size;

            cocoon_t *cocoon = cocoon_lookup(env->cocoons, token);
            if (!cocoon)
            {
                larva_error(env, "Cocoon '%s' was not loaded.", token);
            }
            else
            {
                char *error;
                var (*fn)(bombyx_env_t *, BYTE, var *);

                token2[size++] = '_';
                token2[size] = 0;

                fn = dlsym(cocoon->ptr, token2);

                // restore name
                token2[--size] = 0;

                if ((error = dlerror()) != NULL)
                {
                    fprintf(stderr, "Function '%s' does not exist in cocoon '%s'.\n", token2, token);
                    larva_error(env, 0);
                }

                // pass arguments
                BYTE argc = env->bytecode[env->bc_pos++];

                v1 = fn(env, argc, env->bc_stack + env->bc_stack_size - argc);

                /*
                    If we just decrease stack size then it's a memory leak, but 100% safe.
                    If the stack stays unchanged it grows — may lead to stack overflow.
                    The best option if to clear the stack top manually after the call, but before the result is pushed.
                 */
                while (argc--)
                {
                    var_unset(env, &env->bc_stack[--env->bc_stack_size]);
                }

                if (v1.type == VAR_ERROR)
                {
                    larva_error(env, "%s(): %s", token2, (char *) v1.data);
                }

                stack_push(env, v1);
            }
            break;

            case BCO_BLOCK_DEF:
            size = env->bytecode[env->bc_pos++];
            if (skip_mode)
            {
                env->bc_pos += size;
                break;
            }
            debug_verbose_puts("BCO_BLOCK_DEF");
            memcpy(token, env->bytecode + env->bc_pos, size);
            token[size] = 0;
            env->bc_pos += size;

            if (parent_block)
            {
                char *parent_name = parent_block->name;
                size_t pnl = strlen(parent_name);
                memcpy(token + pnl + 1, token, size);
                memcpy(token, parent_name, pnl);
                token[pnl] = '.';
                token[pnl + 1 + size] = '\0';
            }

            parent_block = block_add(env->blocks, token, env->bc_pos, parent_block);

            // replace the block definition with nulls and skipmode
            memset(env->bytecode + env->bc_pos - size - 2, BCO_IDLE, size + 1);
            env->bytecode[env->bc_pos - 1] = BCO_SKIP;

            // we don't need it now
            skip_mode = 1;
            break;

            case BCO_SKIP:
            skip_mode = 1;
            debug_verbose_puts("BCO_SKIP");
            break;

            case BCO_BLOCK_START:
            if (skip_mode)
            {
                ++level;
                break;
            }
            debug_verbose_puts("BCO_BLOCK_START");
            break;

            case BCO_BLOCK_END:
            if (skip_mode)
            {
                if (--level == 0) skip_mode = 0;
            }
            else
            {
                debug_verbose_puts("BCO_BLOCK_END");

                // if we had WHILE on this level, go back and check it again
                if (env->run_flag[env->gl_level] == RUN_WHILE)
                {
                    env->bc_pos = env->ret_point[env->gl_level--];
                }
                else if (env->run_flag[env->gl_level] == RUN_BLOCK)
                {
                    // clear stack manually
                    stack_clear(env);

                    // push null to stack
                    // TODO: replace 0 with NULL VAR_STRING
                    env->bc_stack[env->bc_stack_size++] = var_as_double(env, 0);
                    parent_block = parent_block->parent;

                    env->bc_pos = env->ret_point[env->gl_level];
                    --env->gl_level;
                    break;
                }
                else
                {
                    // just up (e.g. IF)
                    --env->gl_level;
                }
                // clear stack from garbage
                stack_clear(env);
            }
            break;

            case BCO_CLEAR_STACK:
            debug_verbose_puts("BCO_CLEAR_STACK");
            stack_clear(env);
            break;

            case BCO_REVERSE_STACK:
            debug_verbose_puts("BCO_REVERSE_STACK");
            if (skip_mode)
            {
                ++env->bc_pos;
                break;
            }
            size = env->bytecode[env->bc_pos++];
            size_t iter = size / 2;
            while (iter--)
            {
                op_swap(&env->bc_stack[env->bc_stack_size - 1 - iter], &env->bc_stack[env->bc_stack_size - size + iter]);
            }
            param_count = size;
            break;

            case BCO_RETURN:
            debug_verbose_puts("BCO_RETURN");

            if (env->bc_stack_size >= 1)
            {
                // stack has got something during the RETURN parsing
                v1 = stack_pop(env);

                // just in case, as stack can have more than 1 atom
                stack_clear(env);
            }
            else
            {
                // stack is empty
                v1 = var_as_double(env, 0);
            }

            // no reference can be inherited during return
            v1.ref = NULL;

            env->bc_stack[env->bc_stack_size++] = v1;
            parent_block = parent_block->parent;

            // get da fukk out
            env->bc_pos = env->ret_point[env->gl_level];
            --env->gl_level;
            break;

            case BCO_WHILE:
            debug_verbose_puts("BCO_WHILE");
            ++env->gl_level;

            env->ret_point[env->gl_level] = env->bc_pos - 1;
            env->run_flag[env->gl_level] = RUN_WHILE;
            break;

            case BCO_IF:
            debug_verbose_puts("BCO_IF");
            ++env->gl_level;

            env->run_flag[env->gl_level] = RUN_IF;
            break;

            case BCO_ELSE:
            debug_verbose_puts("BCO_ELSE");

            ++env->gl_level;

            if (env->run_flag[env->gl_level] == RUN_IF) skip_mode = 1;
            break;

            case BCO_CEIT:
            debug_verbose_puts("BCO_CEIT");
            v1 = stack_pop(env);

            if (!var_is_true(env, &v1))
            {
                if (env->run_flag[env->gl_level] == RUN_IF) env->run_flag[env->gl_level] = RUN_ELSE;
                skip_mode = 1;
            }
            var_unset(env, &v1);
            break;

            case BCO_CMP:
            debug_verbose_puts("BCO_CMP");
            v2 = stack_pop(env);
            v1 = stack_pop(env);
            env->bc_stack[env->bc_stack_size++] = var_as_double(env, var_cmp(env, &v1, &v2));
            var_unset(env, &v1);
            var_unset(env, &v2);
            break;

            case BCO_CMP_NOT:
            debug_verbose_puts("BCO_CMP_NOT");
            v2 = stack_pop(env);
            v1 = stack_pop(env);
            env->bc_stack[env->bc_stack_size++] = var_as_double(env, !var_cmp(env, &v1, &v2));
            var_unset(env, &v1);
            var_unset(env, &v2);
            break;

            case BCO_MORE:
            debug_verbose_puts("BCO_MORE");
            v2 = stack_pop(env);
            v1 = stack_pop(env);
            env->bc_stack[env->bc_stack_size++] = var_as_double(env, var_is_more(env, &v1, &v2));
            var_unset(env, &v1);
            var_unset(env, &v2);
            break;

            case BCO_MORE_EQ:
            debug_verbose_puts("BCO_MORE_EQ");
            v2 = stack_pop(env);
            v1 = stack_pop(env);
            env->bc_stack[env->bc_stack_size++] = var_as_double(env, var_is_more_equal(env, &v1, &v2));
            var_unset(env, &v1);
            var_unset(env, &v2);
            break;

            case BCO_LESS:
            debug_verbose_puts("BCO_LESS");
            v2 = stack_pop(env);
            v1 = stack_pop(env);
            env->bc_stack[env->bc_stack_size++] = var_as_double(env, var_is_less(env, &v1, &v2));
            var_unset(env, &v1);
            var_unset(env, &v2);
            break;

            case BCO_LESS_EQ:
            debug_verbose_puts("BCO_LESS_EQ");
            v2 = stack_pop(env);
            v1 = stack_pop(env);
            env->bc_stack[env->bc_stack_size++] = var_as_double(env, var_is_less_equal(env, &v1, &v2));
            var_unset(env, &v1);
            var_unset(env, &v2);
            break;

            case BCO_AND:
            debug_verbose_puts("BCO_AND");
            v2 = stack_pop(env);
            v1 = stack_pop(env);
            op_and(env, &v1, &v2);
            var_unset(env, &v2);
            env->bc_stack[env->bc_stack_size++] = v1;
            break;

            case BCO_OR:
            debug_verbose_puts("BCO_OR");
            v2 = stack_pop(env);
            v1 = stack_pop(env);
            op_or(env, &v1, &v2);
            var_unset(env, &v2);
            env->bc_stack[env->bc_stack_size++] = v1;
            break;

            case BCO_ADD:
            debug_verbose_puts("BCO_ADD");
            v2 = stack_pop(env);
            v1 = stack_pop(env);
            op_add(env, &v1, &v2);
            var_unset(env, &v2);
            env->bc_stack[env->bc_stack_size++] = v1;
            break;

            case BCO_SUB:
            debug_verbose_puts("BCO_SUB");
            v2 = stack_pop(env);
            v1 = stack_pop(env);
            op_subtract(env, &v1, &v2);
            var_unset(env, &v2);
            env->bc_stack[env->bc_stack_size++] = v1;
            break;

            case BCO_MUL:
            debug_verbose_puts("BCO_MUL");
            v2 = stack_pop(env);
            v1 = stack_pop(env);
            op_multiply(env, &v1, &v2);
            var_unset(env, &v2);
            env->bc_stack[env->bc_stack_size++] = v1;
            break;

            case BCO_DIV:
            debug_verbose_puts("BCO_DIV");
            v2 = stack_pop(env);
            v1 = stack_pop(env);
            op_divide(env, &v1, &v2);
            var_unset(env, &v2);
            env->bc_stack[env->bc_stack_size++] = v1;
            break;

            case BCO_POW:
            debug_verbose_puts("BCO_POW");
            v2 = stack_pop(env);
            v1 = stack_pop(env);
            op_power(env, &v1, &v2);
            var_unset(env, &v2);
            env->bc_stack[env->bc_stack_size++] = v1;
            break;

            case BCO_MOD:
            debug_verbose_puts("BCO_MOD");
            v2 = stack_pop(env);
            v1 = stack_pop(env);
            op_modulo(env, &v1, &v2);
            var_unset(env, &v2);
            env->bc_stack[env->bc_stack_size++] = v1;
            break;

            case BCO_INCR:
            debug_verbose_puts("BCO_INCR");
            op_increment(env, &env->bc_stack[env->bc_stack_size - 1]);
            break;

            case BCO_DECR:
            debug_verbose_puts("BCO_DECR");
            op_decrement(env, &env->bc_stack[env->bc_stack_size - 1]);
            break;

            case BCO_INVERT:
            debug_verbose_puts("BCO_INVERT");
            op_invert(env, &env->bc_stack[env->bc_stack_size - 1]);
            break;

            case BCO_UNARY_MINUS:
            debug_verbose_puts("BCO_UNARY_MINUS");
            op_unary_minus(env, &env->bc_stack[env->bc_stack_size - 1]);
            break;

            case BCO_VAR:
            size = env->bytecode[env->bc_pos++];
            if (skip_mode)
            {
                env->bc_pos += size;
                break;
            }
            debug_verbose_puts("BCO_VAR");
            memcpy(token, env->bytecode + env->bc_pos, size);
            token[size] = 0;
            var_add(env->vars, token, VAR_STRING, NULL);
            env->bc_pos += size;
            break;

            case BCO_VARX:
            size = env->bytecode[env->bc_pos++];
            if (skip_mode)
            {
                env->bc_pos += size;
                break;
            }
            debug_verbose_puts("BCO_VARX");
            memcpy(token, env->bytecode + env->bc_pos, size);
            token[size] = 0;
            vt = var_add(env->vars, token, VAR_STRING, NULL);
            env->bc_pos += size;

            v1 = stack_pop(env);
            v1.name = token;

            // v1 was taken from stack and still points on the old var, reset this shit!
            v1.ref = NULL;

            var_sync(env, &v1);
            // this will not unset name, so no worries about the token
            var_unset(env, &v1);
            break;

            case BCO_PARAM:
            size = env->bytecode[env->bc_pos++];
            if (skip_mode)
            {
                env->bc_pos += size;
                break;
            }
            debug_verbose_puts("BCO_PARAM");

            if (!(param_count--))
            {
                larva_error(env, "Function requires more arguments.");
            }

            memcpy(token, env->bytecode + env->bc_pos, size);
            token[size] = 0;
            vt = var_add(env->vars, token, VAR_STRING, NULL);
            op_copy(env, &vt->v, &env->bc_stack[--env->bc_stack_size]);
            env->bc_pos += size;
            break;

            case BCO_PARAMX:
            size = env->bytecode[env->bc_pos++];
            if (skip_mode)
            {
                env->bc_pos += size;
                break;
            }
            debug_verbose_puts("BCO_PARAMX");

            param_count--;

            memcpy(token, env->bytecode + env->bc_pos, size);
            token[size] = 0;
            vt = var_add(env->vars, token, VAR_STRING, NULL);

            // pop the var from stack anyway
            v1 = stack_pop(env);

            // stack may be empty
            if (env->bc_stack_size) v2 = stack_pop(env);
            else memset(&v2, 0, sizeof(var));

            if (v2.type == VAR_UNSET || param_count < 0)
            {
                op_copy(env, &vt->v, &v1);
            }
            else
            {
                op_copy(env, &vt->v, &v2);
            }
            var_unset(env, &v1);
            var_unset(env, &v2);
            env->bc_pos += size;
            break;

            case BCO_PRINT:
            debug_verbose_puts("BCO_PRINT");
            v1 = stack_pop(env);
            var_echo(env, &v1);
            var_unset(env, &v1);
            break;

            case BCO_MICROTIME:
            debug_verbose_puts("BCO_MICROTIME");
            env->bc_stack[env->bc_stack_size++] = var_as_double(env, get_microtime());
            break;

            case BCO_AS_VOID:
            debug_verbose_puts("BCO_AS_VOID");
            memset(&env->bc_stack[env->bc_stack_size++], 0, sizeof(var));
            break;
        }
    }

    stack_clear(env);
}
