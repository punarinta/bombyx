#include "../common.h"
#include "larva.h"
#include "var.h"
#include "block.h"
#include "sys.h"
#include "bytecode.h"

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
        if (bc_stack[bc_stack_size].level < gl_level) break;
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
    size_t size = 0;
    BYTE skip_mode = 0;
    block_t *parent_block = NULL;
    char token[PARSER_MAX_TOKEN_SIZE];

    bc_ready();

    /*puts("=============== BYTECODE =============");
    bc_poo();
    puts("======================================");
exit(0);*/

    started_at = get_microtime();

    // reentry must be here, not above! => rewrite silk function later

    while (bc_pos < bc_length)
    {
        if (bytecode[bc_pos] == BCO_IDLE)
        {
            ++bc_pos;
            continue;
        }

        if (!skip_mode) ++bc_ops;

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
            if (skip_mode) break;
            debug_verbose_puts("BCO_SET");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            if (!v1.name)
            {
            	larva_error("Schnieblie operations are not allowed.");
            }

            op_copy(&v1, &v2);
            var_sync(&v1);
            var_unset(&v2);

            // back to stack
            bc_stack[bc_stack_size++] = v1;
            break;

            case BCO_AS_DOUBLE:
            if (skip_mode)
            {
                bc_pos += sizeof(double);
                break;
            }
            debug_verbose_puts("BCO_AS_DOUBLE");
            memcpy(&d, bytecode + bc_pos, sizeof(double));
            bc_stack[bc_stack_size++] = var_as_double(d);
            bc_pos += sizeof(double);
            break;

            case BCO_AS_STRING:
            size = bytecode[bc_pos] + bytecode[bc_pos + 1] * 256;
            bc_pos += 2;
            if (skip_mode)
            {
                bc_pos += size;
                break;
            }
            debug_verbose_puts("BCO_AS_STRING");
            memcpy(token, bytecode + bc_pos, size);
            token[size] = 0;
            bc_stack[bc_stack_size++] = var_as_string(token, size);
            bc_pos += size;
            break;

            case BCO_CALL:
            if (skip_mode) break;
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

            ++gl_level;

            // we don't need it now
            skip_mode = 1;
            break;

            case BCO_SKIP:
            if (skip_mode) break;
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
                --gl_level;
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
                    // TODO: replace 0 with NULL
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
            }

            // clear stack from garbage
            stack_clear();
            break;

            case BCO_CLEAR_STACK:
            if (skip_mode) break;
            debug_verbose_puts("BCO_CLEAR_STACK");
            stack_clear();
            break;

            case BCO_RETURN:
            if (skip_mode) break;
            debug_verbose_puts("BCO_RETURN");

            if (bc_stack_size > 1)
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

            bc_stack[bc_stack_size++] = v1;
            parent_block = parent_block->parent;

            // get da fukk out
            bc_pos = ret_point[gl_level];
            --gl_level;
            break;

            case BCO_WHILE:
            if (skip_mode) break;
            debug_verbose_puts("BCO_WHILE");
            ++gl_level;

            ret_point[gl_level] = bc_pos - 1;
            run_flag[gl_level] = RUN_WHILE;
            break;

            case BCO_IF:
            if (skip_mode) break;
            debug_verbose_puts("BCO_IF");
            ++gl_level;

            run_flag[gl_level] = RUN_IF;
            break;

            case BCO_ELSE:
            if (skip_mode) break;
            debug_verbose_puts("BCO_ELSE");

            ++gl_level;

            if (run_flag[gl_level] == RUN_IF) skip_mode = 1;
            break;

            case BCO_CEIT:
            if (skip_mode) break;
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
            if (skip_mode) break;
            debug_verbose_puts("BCO_CMP");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            bc_stack[bc_stack_size++] = var_as_double(var_cmp(&v1, &v2));
            var_unset(&v1);
            var_unset(&v2);
            break;

            case BCO_CMP_NOT:
            if (skip_mode) break;
            debug_verbose_puts("BCO_CMP_NOT");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            bc_stack[bc_stack_size++] = var_as_double(!var_cmp(&v1, &v2));
            var_unset(&v1);
            var_unset(&v2);
            break;

            case BCO_MORE:
            if (skip_mode) break;
            debug_verbose_puts("BCO_MORE");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            bc_stack[bc_stack_size++] = var_as_double(var_is_more(&v1, &v2));
            var_unset(&v1);
            var_unset(&v2);
            break;

            case BCO_MORE_EQ:
            if (skip_mode) break;
            debug_verbose_puts("BCO_MORE_EQ");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            bc_stack[bc_stack_size++] = var_as_double(var_is_more_equal(&v1, &v2));
            var_unset(&v1);
            var_unset(&v2);
            break;

            case BCO_LESS:
            if (skip_mode) break;
            debug_verbose_puts("BCO_LESS");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            bc_stack[bc_stack_size++] = var_as_double(var_is_less(&v1, &v2));
            var_unset(&v1);
            var_unset(&v2);
            break;

            case BCO_LESS_EQ:
            if (skip_mode) break;
            debug_verbose_puts("BCO_LESS_EQ");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            bc_stack[bc_stack_size++] = var_as_double(var_is_less_equal(&v1, &v2));
            var_unset(&v1);
            var_unset(&v2);
            break;

            case BCO_AND:
            if (skip_mode) break;
            debug_verbose_puts("BCO_AND");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            op_and(&v1, &v2);
            var_free(&v2);
            bc_stack[bc_stack_size++] = v1;
            break;

            case BCO_OR:
            if (skip_mode) break;
            debug_verbose_puts("BCO_OR");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            op_or(&v1, &v2);
            var_free(&v2);
            bc_stack[bc_stack_size++] = v1;
            break;

            case BCO_ADD:
            if (skip_mode) break;
            debug_verbose_puts("BCO_ADD");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            op_add(&v1, &v2);
            var_unset(&v2);
            bc_stack[bc_stack_size++] = v1;
            break;

            case BCO_SUB:
            if (skip_mode) break;
            debug_verbose_puts("BCO_SUB");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            op_subtract(&v1, &v2);
            var_unset(&v2);
            bc_stack[bc_stack_size++] = v1;
            break;

            case BCO_MUL:
            if (skip_mode) break;
            debug_verbose_puts("BCO_MUL");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            op_multiply(&v1, &v2);
            var_free(&v2);
            bc_stack[bc_stack_size++] = v1;
            break;

            case BCO_DIV:
            if (skip_mode) break;
            debug_verbose_puts("BCO_DIV");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            op_divide(&v1, &v2);
            var_free(&v2);
            bc_stack[bc_stack_size++] = v1;
            break;

            case BCO_INCR:
            if (skip_mode) break;
            debug_verbose_puts("BCO_INCR");
            op_increment(&bc_stack[bc_stack_size - 1]);
            break;

            case BCO_DECR:
            if (skip_mode) break;
            debug_verbose_puts("BCO_DECR");
            op_decrement(&bc_stack[bc_stack_size - 1]);
            break;

            case BCO_INVERT:
            if (skip_mode) break;
            debug_verbose_puts("BCO_INVERT");
            op_invert(&bc_stack[bc_stack_size - 1]);
            break;

            case BCO_UNARY_MINUS:
            if (skip_mode) break;
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
            if (!vt) vt = var_lookup(vars, token);
            bc_pos += size;

            v1 = bc_stack[--bc_stack_size];
            v1.name = token;
            var_sync(&v1);
            // unset does not unset name, so no worries about the token
            var_unset(&v1);
            break;

            case BCO_PRINT:
            if (skip_mode) break;
            debug_verbose_puts("BCO_PRINT");
            v1 = bc_stack[--bc_stack_size];
            var_echo(&v1);
            var_unset(&v1);
            break;

            case BCO_MICROTIME:
            if (skip_mode) break;
            debug_verbose_puts("BCO_MICROTIME");
            bc_stack[bc_stack_size++] = var_as_double(get_microtime());
            break;
        }
    }

    stack_clear();
}
