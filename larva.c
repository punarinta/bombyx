#include "common.h"
#include "larva.h"
#include "core/var.h"
#include "core/block.h"
#include "core/sys.h"

/**
 *  Sets up the processor
 */
void larva_init(char *incoming_code, unsigned int len)
{
    vars_count   = MIN_VARIABLES;
    blocks_count = MIN_BLOCKS;
    vars         = var_table_create(MIN_VARIABLES);
    blocks       = block_table_create(MIN_BLOCKS);

    code_pos = 0;
    code_length = 0;
    code = malloc(len * sizeof(char));

    // TODO: include all the necessary files

    for (unsigned int i = 0; i < len; i++)
    {
        if (i < len - 1 && incoming_code[i] == '\\')
        {
            if (incoming_code[i + 1] == 'n') code[code_length] = '\n';
            else if (incoming_code[i + 1] == 't') code[code_length] = '\t';
            else code_length--;
            i++;
        }
        else if (incoming_code[i] == '#')
        {
            if (i < len - 1 && incoming_code[i + 1] == '#')
            {
                while (i++ < len) if (incoming_code[i] == '#' && incoming_code[i + 1] == '#') {i++; break;}
            }
            else while (i++ < len) if (incoming_code[i] == '\n') break;
            
            continue;
        }
        else
        {
            code[code_length] = incoming_code[i];
        }

        code_length++;
    }

    started_at = get_microtime();

    larva_map_blocks();
}


/**
 *  Allocates more space for variables
 */
void larva_grow(unsigned long size)
{
/*    if (!size)
    {
        // by default just double it
        size = MIN_VARIABLES;
    }

    vars = realloc(vars, sizeof(var) * (vars_count + size));

    if (!vars) larva_stop(ERR_NO_MEMORY);
    vars_count += size;*/
}

int larva_digest_start()
{
    gl_error = 0;
    gl_level = 0;
    run_flag[0] = 0;

    setjmp(error_exit);

    if (gl_error) return larva_stop(gl_error);

    // no vars need to leave to the outer world
    var_free(larva_digest());

    return larva_stop(0);
}

void larva_map_blocks()
{
    // TODO: make this shit dynamic
    char token[PARSER_MAX_TOKEN_SIZE];
    block_t *parent_block = NULL;
    int not_allowed = 0;

    while (code[code_pos])
    {
        if (isspace(code[code_pos])) { code_pos++; continue; }

        read_token(token);

        if (!strcmp(token, "block"))
        {
            read_token(token);

            if (not_allowed)
            {
                fprintf(stderr, "Block '%s' is declared within control statement.", token);
                larva_error(code_pos);
            }

            // if this block haz parents, copy parent_name before own name
            if (parent_block)
            {
                size_t tl = strlen(token);
                char *parent_name = parent_block->name;
                memcpy(token + strlen(parent_name) + 1, token, tl);
                memcpy(token, parent_name, strlen(parent_name));
                token[strlen(parent_name)] = '.';
                token[strlen(parent_name) + 1 + tl] = '\0';
            }

            // check what's the status of this var
            if (block_lookup(blocks, token))
            {
                fprintf(stderr, "Block '%s' already exists.", token);
                larva_error(code_pos);
            }

            // scan for '{'
            while (code[code_pos]) if (code[code_pos++] == '{') break;

            parent_block = block_add(blocks, token, code_pos, parent_block);
        }
        else if (!strcmp(token, "if") || !strcmp(token, "else") )
        {
            not_allowed++;
            while (code[code_pos]) if (code[code_pos++] == '{') break;
        }
        else if (!strcmp(token, "}"))
        {
            if (not_allowed) not_allowed--;
            else parent_block = parent_block->parent;
        }
    }

    code_pos = 0;
}

/**
 *  Processes code buffer
 */
var *larva_digest()
{
    var_t *token_var = NULL;
    char token[PARSER_MAX_TOKEN_SIZE];
    char oper[PARSER_MAX_TOKEN_SIZE];
    var *r;

    while (code[code_pos])
    {
        // find first significant character
        if (isspace(code[code_pos]))
        {
            code_pos++;
            continue;
        }

        size_t line_start = code_pos;
        read_token(token);

        if (!strcmp(token, "var"))
        {
            re_read_var:

            read_token(token);

            // check what's the status of this var
            token_var = var_lookup(vars, token);

            if (token_var)
            {
                fprintf(stderr, "Variable '%s' already exists.", token);
                larva_error(code_pos);
            }

            // TODO: check that token is not in the list of reserved words

            // should be no var -- create it
            token_var = var_add(vars, token, VAR_STRING, NULL);

            // variable is just initialized, but not defined
            if (code[code_pos] == '\n') continue;
            
            // expect operator '='
            read_token(oper);

            if (!strlen(oper)) continue;

            if (!strcmp(oper, ","))
            {
                goto re_read_var;
            }
            else if (strcmp(oper, "="))
            {
                fprintf(stderr, "Operator '=' expected, found '%s'", oper);
                larva_error(code_pos);
            }

            // equalize
            var *parse_result = parse();

            if (!parse_result)
            {
                fprintf(stderr, "Operator '=' expects an value to follow");
                larva_error(code_pos);
            }

            if (parse_result->name) free(parse_result->name);
            parse_result->name = strdup(token);
            var_sync(parse_result);
            var_free(parse_result);

            // we have one more var to init
            if (code[code_pos] == ',')
            {
                code_pos++;
                goto re_read_var;
            }
        }
        else if (!strcmp(token, "return"))
        {
            if (gl_level == 0)
            {
                if (verbose) fprintf(stdout, "Returning from zero level. Bye-bye!\n");
                return NULL;
            }

            r = parse();

            // this var CANNOT have a name
            if (r->name)
            {
                free(r->name);
                r->name = NULL;
            }

            if (verbose) fprintf(stdout, "Returning from level %u...\n", gl_level);

            return r;
        }
        else if (!strcmp(token, "block")) skip_block();
        else if (!strcmp(token, "if"))
        {
            unsigned long expr_start = code_pos, level = 0;
            // find expression
            while (code[code_pos])
            {
                if (code[code_pos] == '(') level++;
                if (code[code_pos] == ')')
                {
                    level--;
                    if (level < 1) { code_pos++; break; }
                }
                code_pos++;
            }

            char *expr = calloc(code_pos - expr_start + 1, sizeof(char));
            memcpy(expr, &code[expr_start], code_pos - expr_start + 1);
            expr[code_pos - expr_start] = '\0';

            var *x = parse_expression(expr);
            free(expr);

            // compare and unset 'x'
            if (!var_to_double(x))
            {
                run_flag[gl_level + 1] = 2;  // RUN_ELSE
                skip_block();
            }
            else
            {
                // find where if-block begins
                while (code[code_pos]) if (code[code_pos++] == '{') break;

                // start running this block
                run_flag[++gl_level] = 1;  // RUN_THIS
            }
        }
        else if (!strcmp(token, "{"))
        {
            fprintf(stderr, "Blocks should be named or preceeded by control statements.");
            larva_error(code_pos);
        }
        else if (!strcmp(token, "}"))
        {
            // if you met a bracket and 'run_flag' on this level is zero, then this is a function end
            if (!run_flag[gl_level])
            {
                return NULL;
            }
            else gl_level--;
        }
        else if (!strcmp(token, "else"))
        {
            if (run_flag[gl_level + 1] == 1) skip_block();
            else
            {
                // find where else-block begins
                while (code[code_pos]) if (code[code_pos++] == '{') break;
                run_flag[++gl_level] = 1;
            }
        }
        else
        {
            code_pos = line_start;
            // result is not fed anywhere, free it
            var_free(parse());
        }
    }

    return NULL;
}

void read_token(char *token)
{
    size_t start = code_pos;

    // read until newline
    while (code[code_pos] != '\0')
    {
        token[code_pos - start] = code[code_pos];
        code_pos++;
        // either it's a function call or just a command
        if (code[code_pos] == '(' || code[code_pos] == ',' || isspace(code[code_pos])) break;
    }

    token[code_pos - start] = '\0';
    trim(token);
}

void skip_block()
{
    BYTE level = 0;
    while (code[code_pos])
    {
        if (code[code_pos] == '{') level++;
        if (code[code_pos] == '}') if (--level < 1)
        {
            code_pos++;
            break;
        }
        code_pos++;
    }
}

void larva_error()
{
    int line = 1, sym = 0; size_t i = 0;
    while (code[i] != '\0')
    {
        i++;
        sym++;
        if (code[i] == 10 || code[i] == 13)
        {
            line++;
            sym = 0;
        }

        if (i == code_pos) break;
    }

    fprintf(stderr, "\nError on line %d, sym %d.\n\n", line, sym);

    gl_error = 1;
    longjmp(error_exit, 1);
}

/**
 *  Call to stop execution
 */
int larva_stop(int ret_code)
{
    if (verbose)
    {
        fputs("================= DUMP ===============", stdout);
        larva_poo();
        fputs("\n======================================\n", stdout);
    }

    var_table_delete(vars);
    block_table_delete(blocks);

    if (code) free(code);

#ifndef __APPLE__
    muntrace();
#endif

    return ret_code;
}

void larva_poo()
{
    unsigned int i;
    char types[][20] = {"UNSET", "BYTE", "WORD", "DWORD", "QWORD", "FLOAT", "DOUBLE", "STRING", "BLOCK", "ARRAY"};

    var_t *v_list;
    block_t *b_list;

    for (i = 0; i < vars->size; i++)
    {
        for (v_list = vars->table[i]; v_list != NULL; v_list = v_list->next)
        {
            fprintf(stdout, "\n'%s' [%s of size %u] = ", v_list->name, types[v_list->type], v_list->data_size);

            var *v = var_as_var_t(v_list);
            var_echo(v);
            var_free(v);
        }
    }

    for (i = 0; i < blocks->size; i++)
    {
        for (b_list = blocks->table[i]; b_list != NULL; b_list = b_list->next)
        {
            if (b_list->pos) fprintf(stdout, "\nBlock '%s' at pos %u", b_list->name, b_list->pos);
        }
    }
}
