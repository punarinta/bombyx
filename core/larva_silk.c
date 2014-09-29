#include "../common.h"
#include "larva.h"
#include "var.h"
#include "block.h"
#include "sys.h"
#include "bytecode.h"

// execute byte-code
void larva_silk()
{
    double d;
    var v1, v2;
    var_t *vt;
    size_t size = 0;
    char token[PARSER_MAX_TOKEN_SIZE];
    BYTE level = 0;
    BYTE skip_mode = 0;
    gl_level = 0;
    char temp_error[256];
    block_t *parent_block = NULL;

    bc_ready();

   /* puts("=============== BYTECODE =============");
    bc_poo();
    puts("======================================");
exit(0);*/

    started_at = get_microtime();

    // reentry must be here, not above! => rewrite silk function later

    while (bc_pos < bc_length)
    {
        switch (bytecode[bc_pos++])
        {
            case BCO_AS_VAR:
            size = bytecode[bc_pos++];
            if (skip_mode)
            {
                bc_pos += size;
                break;
            }
            if (verbose) puts("BCO_AS_VAR");
            memcpy(token, bytecode + bc_pos, size);
            token[size] = 0;
            vt = var_lookup(vars, token);
            bc_stack[bc_stack_size++] = var_as_var_t(vt);
            bc_pos += size;
            break;

            case BCO_SET:
            if (skip_mode) break;
            if (verbose) puts("BCO_SET");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            if (!v1.name)
            {
            	larva_error("Schnieblie operations are not allowed.");
            }

            // copy everything except name from v1 to v0
            op_copy(&v1, &v2);
            var_sync(&v1);
            var_unset(&v2);
            // back to stack
            bc_stack[bc_stack_size++] = v1;
            break;

            case BCO_AS_DOUBLE:
            if (skip_mode)
            {
                bc_pos += 8;
                break;
            }
            if (verbose) puts("BCO_AS_DOUBLE");
            memcpy(&d, bytecode + bc_pos, 8);
            bc_stack[bc_stack_size++] = var_as_double(d);
            bc_pos += 8;
            break;

            case BCO_AS_STRING:
            size = bytecode[bc_pos + 1] + bytecode[bc_pos + 2] * 256;
            bc_pos += 2;
            if (skip_mode)
            {
                bc_pos += size;
                break;
            }
            if (verbose) puts("BCO_AS_STRING");
            memcpy(token, bytecode + bc_pos, size);
            token[size] = 0;
            bc_stack[bc_stack_size++] = var_as_string(token);
            bc_pos += size;
            break;

            case BCO_CALL:
            if (skip_mode) break;
            if (verbose) puts("BCO_CALL");
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
                ret_point[gl_level++] = bc_pos;
            	// step into
            	run_flag[gl_level] = 0;
            	bc_pos = this_block->pos;
            }
            break;

            case BCO_BLOCK:
            if (skip_mode) break;
            if (verbose) puts("BCO_BLOCK");
            size = bytecode[bc_pos++];
            memcpy(token, bytecode + bc_pos, size);
            token[size] = 0;
            bc_pos += size;
            parent_block = block_add(blocks, token, bc_pos, parent_block);

            ++gl_level;
            // we don't need it now
            skip_mode = 1;
            break;

            case BCO_BLOCK_START:
            if (skip_mode)
            {
                ++level;
            }
            if (skip_mode) break;
            if (verbose) puts("BCO_BLOCK_START");
            break;

            case BCO_BLOCK_END:
            if (verbose) puts("BCO_BLOCK_END");
            if (skip_mode)
            {
                if (--level == 0) skip_mode = 0;
                --gl_level;
            }
            else
            {
                // if we had WHILE on this level, go back and check it again
                if (run_flag[gl_level] == 3)
                {
                    bc_pos = ret_point[gl_level--];
                }
                else
                {
                    // just up?
                    --gl_level;
                }
            }

            parent_block = parent_block->parent;

            // clear stack from garbage
            while (bc_stack_size) var_unset(&bc_stack[--bc_stack_size]);
            break;

            case BCO_WHILE:
            if (skip_mode) break;
            if (verbose) puts("BCO_WHILE");
            ++gl_level;

            ret_point[gl_level] = bc_pos - 1;
            run_flag[gl_level] = 3;  // RUN_WHILE
            break;

            case BCO_IF:
            if (skip_mode) break;
            if (verbose) puts("BCO_IF");
            ++gl_level;

            run_flag[gl_level] = 1;  // RUN_IF
            break;

            case BCO_ELSE:
            if (skip_mode) break;
            if (verbose) puts("BCO_ELSE");

            ++gl_level;

            if (run_flag[gl_level] == 2)
            {

            }
            else skip_mode = 1;
            break;

            case BCO_CEIT:
            if (skip_mode) break;
            if (verbose) puts("BCO_CEIT");
            v1 = bc_stack[--bc_stack_size];

            if (!var_extract_double(&v1))
            {
                if (run_flag[gl_level] == 1) run_flag[gl_level] = 2;    // RUN_ELSE
                skip_mode = 1;
            }
            var_unset(&v1);
            break;

            case BCO_CMP:
            if (skip_mode) break;
            if (verbose) puts("BCO_CMP");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            bc_stack[bc_stack_size++] = var_as_double(var_cmp(&v1, &v2));
            var_unset(&v1);
            var_unset(&v2);
            break;

            case BCO_CMP_NOT:
            if (skip_mode) break;
            if (verbose) puts("BCO_CMP_NOT");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            bc_stack[bc_stack_size++] = var_as_double(!var_cmp(&v1, &v2));
            var_unset(&v1);
            var_unset(&v2);
            break;

            case BCO_ADD:
            if (skip_mode) break;
            if (verbose) puts("BCO_ADD");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            op_add(&v1, &v2);
            var_unset(&v2);
            bc_stack[bc_stack_size++] = v1;
            break;

            case BCO_SUB:
            if (skip_mode) break;
            if (verbose) puts("BCO_SUB");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            op_subtract(&v1, &v2);
            var_unset(&v2);
            bc_stack[bc_stack_size++] = v1;
            break;

            case BCO_MUL:
            if (skip_mode) break;
            if (verbose) puts("BCO_MUL");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            op_multiply(&v1, &v2);
            var_free(&v2);
            bc_stack[bc_stack_size++] = v1;
            break;

            case BCO_DIV:
            if (skip_mode) break;
            if (verbose) puts("BCO_DIV");
            v2 = bc_stack[--bc_stack_size];
            v1 = bc_stack[--bc_stack_size];
            op_divide(&v1, &v2);
            var_free(&v2);
            bc_stack[bc_stack_size++] = v1;
            break;

            case BCO_INCR:
            if (skip_mode) break;
            if (verbose) puts("BCO_INCR");
            op_increment(&bc_stack[bc_stack_size - 1]);
            break;

            case BCO_DECR:
            if (skip_mode) break;
            if (verbose) puts("BCO_DECR");
            op_decrement(&bc_stack[bc_stack_size - 1]);
            break;

            case BCO_VAR:
            size = bytecode[bc_pos++];
            if (skip_mode)
            {
                bc_pos += size;
                break;
            }
            memcpy(token, bytecode + bc_pos, size);
            token[size + 1] = 0;
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
            if (verbose) puts("BCO_VARX");
            memcpy(token, bytecode + bc_pos, size);
            token[size] = 0;
            vt = var_add(vars, token, VAR_STRING, NULL);
            if (!vt) vt = var_lookup(vars, token);
            bc_pos += size;

            v1 = bc_stack[--bc_stack_size];
            v1.name = strdup(token);
            var_sync(&v1);
            var_unset(&v1);
            // unset does not unset name
            free(v1.name);
            break;

            case BCO_PRINT:
            if (skip_mode) break;
            v1 = bc_stack[--bc_stack_size];
            var_echo(&v1);
            var_unset(&v1);
            break;

            case BCO_MICROTIME:
            if (skip_mode) break;
            if (verbose) puts("BCO_MICROTIME");
            bc_stack[bc_stack_size++] = var_as_double(get_microtime());
            break;
        }
    }
}
