#include "../common.h"
#include "larva.h"
#include "var.h"
#include "block.h"
#include "sys.h"
#include "bytecode.h"
#include "../vendor/jansson.h"

void stack_push(var v)
{
    v.level = gl_level;
    bc_stack[bc_stack_size++] = v;
}

/*
    Must be called before the level is left
*/
void stack_clear()
{
    while (bc_stack_size)
    {
        if (bc_stack[bc_stack_size - 1].level < gl_level) return;
        var_unset(&bc_stack[--bc_stack_size]);
    }
}

// execute byte-code
void larva_silk()
{
    double d;
    var_t *vt;
    var v1, v2;
    gl_level = 0;
    BYTE level = 0;
    run_flag[0] = 0;    // do we need this?
    size_t size = 0;
    BYTE skip_mode = 0;
    size_t param_count = 0;
    block_t *parent_block = NULL;
    char token[PARSER_MAX_TOKEN_SIZE];

    bc_ready();

/*    puts("=============== BYTECODE =============");
    bc_poo();
    puts("======================================");*/
//exit(0);

    started_at = get_microtime();

    // reentry must be here, not above! => rewrite silk function later

    while (bc_pos < bc_length)
    {
        /*if (bytecode[bc_pos] == BCO_IDLE)
        {
            ++bc_pos;
            continue;
        }*/

        if (skip_mode)
        {
            if (bytecode[bc_pos] & IS_BCO_SKIPPABLE)
            {
                // bytecode >=32 --> can be skipped right now
                ++bc_pos;
                continue;
            }
        }
#ifdef BOMBYX_DEBUG
        else
        {
            ++bc_ops;
        }
#endif

        switch (bytecode[bc_pos++])
        {
            case BCO_AS_VAR:
            size = bytecode[bc_pos++];
            if (skip_mode)
            {
                bc_pos += size;
                break;
            }
            debug_verbose_puts("BCO_AS_VAR");
            memcpy(token, bytecode + bc_pos, size);
            token[size] = 0;
            vt = var_lookup(vars, token);
            bc_stack[bc_stack_size++] = var_as_var_t(vt);
            bc_pos += size;
            break;

            case BCO_SET:
            debug_verbose_puts("BCO_SET");
            v2 = bc_stack[--bc_stack_size];
            if (!bc_stack[bc_stack_size - 1].ref)
            {
            	larva_error("Left part of an equation should be a variable.");
            }

            // note: v1 stays inside the stack

            op_copy(&((var_t *)bc_stack[bc_stack_size - 1].ref)->v, &v2);
            var_unset(&v2);
            break;

            case BCO_AS_DOUBLE:
            if (skip_mode)
            {
                bc_pos += sizeof(double);
                break;
            }
            debug_verbose_puts("BCO_AS_DOUBLE");
            d = *(double *)(bytecode + bc_pos);
            bc_stack[bc_stack_size++] = var_as_double(d);
            bc_pos += sizeof(double);
            break;

            case BCO_AS_STRING:
            size = bytecode[bc_pos] + (bytecode[bc_pos + 1] << 8);
            bc_pos += 2;
            if (skip_mode)
            {
                bc_pos += size;
                break;
            }
            debug_verbose_puts("BCO_AS_STRING");
            memcpy(token, bytecode + bc_pos, size);
            token[size] = 0;
            stack_push(var_as_string(token, size));
            bc_pos += size;
            break;

            case BCO_FROM_JSON:
            size = bytecode[bc_pos] + (bytecode[bc_pos + 1] << 8);
            bc_pos += 2;
            if (skip_mode)
            {
                bc_pos += size;
                break;
            }
            debug_verbose_puts("BCO_FROM_JSON");
            char *str = malloc(size + 1);
            memcpy(str, bytecode + bc_pos, size);
            str[size] = 0;
            bc_pos += size;
            stack_push(var_from_json(str));
            free(str);
            break;

            case BCO_ACCESS:
            size = bytecode[bc_pos++];
            if (skip_mode)
            {
                bc_pos += size;
                break;
            }
            debug_verbose_puts("BCO_ACCESS");
            memcpy(token, bytecode + bc_pos, size);
            token[size] = 0;

            v1 = bc_stack[--bc_stack_size];
            if (v1.type != VAR_STRING)
            {
                larva_error("APath key must be a string.");
            }

            // TODO: parse APath and access JSON recursively

            vt = var_lookup(vars, token);
            if (vt->v.type != VAR_MAP && vt->v.type != VAR_ARRAY)
            {
                fprintf(stderr, "Object '%s' is not accessible with [] operator.", token);
                larva_error(0);
            }

        /*    json_t *jt = vt->v.data;

            // get JSON type
            if (json_typeof(jt) == JSON_OBJECT)
            {
                jt = json_object_get(jt, v1.data);
            }
            else if (json_typeof(jt) == JSON_ARRAY)
            {
                // TODO: convert token to int
                jt = json_array_get(jt, 0);
            }
            else
            {
                fprintf(stderr, "JSON-object '%s' is scalar.", token);
                larva_error(0);
            }

            var_unset(&v1);

            v1 = var_as_json(jt);*/
            stack_push(v1);
            break;

            case BCO_CALL:
            debug_verbose_puts("BCO_CALL");
            size = bytecode[bc_pos++];
            if (skip_mode)
            {
                bc_pos += size;
                break;
            }
            memcpy(token, bytecode + bc_pos, size);
            token[size] = 0;
            bc_pos += size;
            block_t *this_block = block_lookup(blocks, token);
            if (!this_block)
            {
                sprintf(temp_error, "Unknown function '%s'.", token);
                larva_error(temp_error);
            }
            else
            {
                ++gl_level;
                ret_point[gl_level] = bc_pos;
            	// step into
            	run_flag[gl_level] = RUN_BLOCK;
            	bc_pos = this_block->pos;
            }
            break;

            case BCO_BLOCK_DEF:
            size = bytecode[bc_pos++];
            if (skip_mode)
            {
                bc_pos += size;
                break;
            }
            debug_verbose_puts("BCO_BLOCK_DEF");
            memcpy(token, bytecode + bc_pos, size);
            token[size] = 0;
            bc_pos += size;

            if (parent_block)
            {
                char *parent_name = parent_block->name;
                memcpy(token + strlen(parent_name) + 1, token, size);
                memcpy(token, parent_name, strlen(parent_name));
                token[strlen(parent_name)] = '.';
                token[strlen(parent_name) + 1 + size] = '\0';
            }

            parent_block = block_add(blocks, token, bc_pos, parent_block);

            // replace the block definition with nulls and skipmode
            memset(bytecode + bc_pos - size - 2, BCO_IDLE, size + 1);
            bytecode[bc_pos - 1] = BCO_SKIP;

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
                if (run_flag[gl_level] == RUN_WHILE)
                {
                    bc_pos = ret_point[gl_level--];
                }
                else if (run_flag[gl_level] == RUN_BLOCK)
                {
                    // clear stack manually
                    stack_clear();

                    // push null to stack
                    // TODO: replace 0 with NULL VAR_STRING
                    bc_stack[bc_stack_size++] = var_as_double(0);
                    parent_block = parent_block->parent;

                    bc_pos = ret_point[gl_level];
                    --gl_level;
                    break;
                }
                else
                {
                    // just up (e.g. IF)
                    --gl_level;
                }
                // clear stack from garbage
                stack_clear();
            }
            break;

            case BCO_CLEAR_STACK:
            debug_verbose_puts("BCO_CLEAR_STACK");
            stack_clear();
            break;

            case BCO_REVERSE_STACK:
            debug_verbose_puts("BCO_REVERSE_STACK");
            if (skip_mode)
            {
                ++bc_pos;
                break;
            }
            size = bytecode[bc_pos++];
            size_t iter = size / 2;
            while (iter--)
            {
                op_swap(&bc_stack[bc_stack_size - 1 - iter], &bc_stack[bc_stack_size - size + iter]);
            }
            param_count = size;
            break;

            case BCO_RETURN:
            debug_verbose_puts("BCO_RETURN");

            if (bc_stack_size >= 1)
            {
                // stack has got something during the RETURN parsing
                v1 = bc_stack[--bc_stack_size];

                // just in case, as stack can have more than 1 atom
                stack_clear();
            }
            else
            {
                // stack is empty
                v1 = var_as_double(0);
            }

            // no reference can be inherited during return
            v1.ref = NULL;

            bc_stack[bc_stack_size++] = v1;
            parent_block = parent_block->parent;

            // get da fukk out
            bc_pos = ret_point[gl_level];
            --gl_level;
            break;

            case BCO_WHILE:
            debug_verbose_puts("BCO_WHILE");
            ++gl_level;

            ret_point[gl_level] = bc_pos - 1;
            run_flag[gl_level] = RUN_WHILE;
            break;

            case BCO_IF:
            debug_verbose_puts("BCO_IF");
            ++gl_level;

            run_flag[gl_level] = RUN_IF;
            break;

            case BCO_ELSE:
            debug_verbose_puts("BCO_ELSE");

            ++gl_level;

            if (run_flag[gl_level] == RUN_IF) skip_mode = 1;
            break;

            case BCO_CEIT:
            debug_verbose_puts("BCO_CEIT");
            v1 = bc_stack[--bc_stack_size];

            if (!var_is_true(&v1))
            {
                if (run_flag[gl_level] == RUN_IF) run_flag[gl_level] = RUN_ELSE;
                skip_mode = 1;
            }
            var_unset(&v1);
            break;

            case BCO_CMP:
            debug_verbose_puts("BCO_CMP");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            bc_stack[bc_stack_size++] = var_as_double(var_cmp(&v1, &v2));
            var_unset(&v1);
            var_unset(&v2);
            break;

            case BCO_CMP_NOT:
            debug_verbose_puts("BCO_CMP_NOT");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            bc_stack[bc_stack_size++] = var_as_double(!var_cmp(&v1, &v2));
            var_unset(&v1);
            var_unset(&v2);
            break;

            case BCO_MORE:
            debug_verbose_puts("BCO_MORE");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            bc_stack[bc_stack_size++] = var_as_double(var_is_more(&v1, &v2));
            var_unset(&v1);
            var_unset(&v2);
            break;

            case BCO_MORE_EQ:
            debug_verbose_puts("BCO_MORE_EQ");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            bc_stack[bc_stack_size++] = var_as_double(var_is_more_equal(&v1, &v2));
            var_unset(&v1);
            var_unset(&v2);
            break;

            case BCO_LESS:
            debug_verbose_puts("BCO_LESS");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            bc_stack[bc_stack_size++] = var_as_double(var_is_less(&v1, &v2));
            var_unset(&v1);
            var_unset(&v2);
            break;

            case BCO_LESS_EQ:
            debug_verbose_puts("BCO_LESS_EQ");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            bc_stack[bc_stack_size++] = var_as_double(var_is_less_equal(&v1, &v2));
            var_unset(&v1);
            var_unset(&v2);
            break;

            case BCO_AND:
            debug_verbose_puts("BCO_AND");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            op_and(&v1, &v2);
            var_unset(&v2);
            bc_stack[bc_stack_size++] = v1;
            break;

            case BCO_OR:
            debug_verbose_puts("BCO_OR");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            op_or(&v1, &v2);
            var_unset(&v2);
            bc_stack[bc_stack_size++] = v1;
            break;

            case BCO_ADD:
            debug_verbose_puts("BCO_ADD");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            op_add(&v1, &v2);
            var_unset(&v2);
            bc_stack[bc_stack_size++] = v1;
            break;

            case BCO_SUB:
            debug_verbose_puts("BCO_SUB");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            op_subtract(&v1, &v2);
            var_unset(&v2);
            bc_stack[bc_stack_size++] = v1;
            break;

            case BCO_MUL:
            debug_verbose_puts("BCO_MUL");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            op_multiply(&v1, &v2);
            var_unset(&v2);
            bc_stack[bc_stack_size++] = v1;
            break;

            case BCO_DIV:
            debug_verbose_puts("BCO_DIV");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            op_divide(&v1, &v2);
            var_unset(&v2);
            bc_stack[bc_stack_size++] = v1;
            break;

            case BCO_INCR:
            debug_verbose_puts("BCO_INCR");
            op_increment(&bc_stack[bc_stack_size - 1]);
            break;

            case BCO_DECR:
            debug_verbose_puts("BCO_DECR");
            op_decrement(&bc_stack[bc_stack_size - 1]);
            break;

            case BCO_INVERT:
            debug_verbose_puts("BCO_INVERT");
            op_invert(&bc_stack[bc_stack_size - 1]);
            break;

            case BCO_UNARY_MINUS:
            debug_verbose_puts("BCO_UNARY_MINUS");
            op_unary_minus(&bc_stack[bc_stack_size - 1]);
            break;

            case BCO_VAR:
            size = bytecode[bc_pos++];
            if (skip_mode)
            {
                bc_pos += size;
                break;
            }
            debug_verbose_puts("BCO_VAR");
            memcpy(token, bytecode + bc_pos, size);
            token[size] = 0;
            var_add(vars, token, VAR_STRING, NULL);
            bc_pos += size;
            break;

            case BCO_VARX:
            size = bytecode[bc_pos++];
            if (skip_mode)
            {
                bc_pos += size;
                break;
            }
            debug_verbose_puts("BCO_VARX");
            memcpy(token, bytecode + bc_pos, size);
            token[size] = 0;
            vt = var_add(vars, token, VAR_STRING, NULL);
            bc_pos += size;

            v1 = bc_stack[--bc_stack_size];
            v1.name = token;
            var_sync(&v1);
            // this will not unset name, so no worries about the token
            var_unset(&v1);
            break;

            case BCO_PARAM:
            size = bytecode[bc_pos++];
            if (skip_mode)
            {
                bc_pos += size;
                break;
            }
            debug_verbose_puts("BCO_PARAM");

            if (!(param_count--))
            {
                larva_error("Block was called with less arguments than you are fetching.");
            }

            memcpy(token, bytecode + bc_pos, size);
            token[size] = 0;
            vt = var_add(vars, token, VAR_STRING, NULL);
            op_copy(&vt->v, &bc_stack[--bc_stack_size]);
            bc_pos += size;
            break;

            case BCO_PARAMX:
            size = bytecode[bc_pos++];
            if (skip_mode)
            {
                bc_pos += size;
                break;
            }
            debug_verbose_puts("BCO_PARAMX");

            if (!(param_count--))
            {
                larva_error("Block was called with less arguments than you are fetching.");
            }

            memcpy(token, bytecode + bc_pos, size);
            token[size] = 0;
            vt = var_add(vars, token, VAR_STRING, NULL);

            // pop the var from stack anyway
            v1 = bc_stack[--bc_stack_size];
            v2 = bc_stack[--bc_stack_size];

            if (v2.type == VAR_UNSET)
            {
                op_copy(&vt->v, &v1);
            }
            else
            {
                op_copy(&vt->v, &v2);
            }
            var_unset(&v1);
            var_unset(&v2);
            bc_pos += size;
            break;

            case BCO_PRINT:
            debug_verbose_puts("BCO_PRINT");
            v1 = bc_stack[--bc_stack_size];
            var_echo(&v1);
            var_unset(&v1);
            break;

            case BCO_MICROTIME:
            debug_verbose_puts("BCO_MICROTIME");
            bc_stack[bc_stack_size++] = var_as_double(get_microtime());
            break;

            case BCO_AS_VOID:
            debug_verbose_puts("BCO_AS_VOID");
            memset(&bc_stack[bc_stack_size++], 0, sizeof(var));
            break;
        }
    }

    stack_clear();
}
