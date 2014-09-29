#include "../common.h"
#include "larva.h"
#include "var.h"
#include "block.h"
#include "sys.h"
#include "bytecode.h"

/**
 *  Sets up the processor
 */
void larva_init(char *incoming_code, unsigned int len)
{
    vars_count   = MIN_VARIABLES;
    blocks_count = MIN_BLOCKS;
    vars         = var_table_create(MIN_VARIABLES);
    blocks       = block_table_create(MIN_BLOCKS);

    gl_level = 0;
    run_flag[0] = 0;

    code_pos = 0;
    code_length = 0;
    code = malloc(len * sizeof(char));
    BYTE quotes_on = 0;

    bc_init();

    // TODO: include all the necessary files

    for (unsigned int i = 0; i < len; ++i)
    {
        if (incoming_code[i] == '\'') quotes_on = !quotes_on;

        if (i < len - 1 && incoming_code[i] == '\\')
        {
            if (incoming_code[i + 1] == 'n') code[code_length] = '\n';
            else if (incoming_code[i + 1] == 't') code[code_length] = '\t';
            else code_length--;
            ++i;
        }
        else if (incoming_code[i] == '#' && !quotes_on)
        {
            if (i < len - 1 && incoming_code[i + 1] == '#')
            {
                while (i++ < len) if (incoming_code[i] == '#' && incoming_code[i + 1] == '#') {++i; break;}
            }
            else while (i++ < len) if (incoming_code[i] == '\n') break;
            
            continue;
        }
        else
        {
            code[code_length] = incoming_code[i];
        }

        ++code_length;
    }

    larva_chew();
}

void larva_chew()
{
    int not_allowed = 0;
    char token[PARSER_MAX_TOKEN_SIZE];

    while (code[code_pos])
    {
        if (isspace(code[code_pos])) { ++code_pos; continue; }

        larva_read_token(token);

        if (!strcmp(token, "block"))
        {
            larva_read_token(token);

            if (not_allowed)
            {
                fprintf(stderr, "Block '%s' is declared within control statement.", token);
                larva_error(0);
            }

            /*// if this block haz parents, copy parent_name before own name
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
                larva_error(0);
            }*/

            // scan for '{'
            while (code[code_pos]) if (code[code_pos++] == '{') break;
        }
        else if (!strcmp(token, "use"))
        {
            larva_read_token(token);

            void *lib_handle;
            double (*fn)(int *);
            int x;
            char *error;

            lib_handle = dlopen(token, RTLD_LAZY);

            if (!lib_handle)
            {
                fprintf(stderr, "%s\n", dlerror());
                larva_error(0);
            }

            fn = dlsym(lib_handle, "dynamic_test");
            if ((error = dlerror()) != NULL)
            {
                fprintf(stderr, "%s\n", error);
                larva_error(0);
            }

            (*fn)(&x);
            printf("val = %d\n", x);

            dlclose(lib_handle);
        }
        else if (!strcmp(token, "if") || !strcmp(token, "else") || !strcmp(token, "while") )
        {
            ++not_allowed;
            while (code[code_pos]) if (code[code_pos++] == '{') break;
        }
        else if (!strcmp(token, "}"))
        {
            if (not_allowed) --not_allowed;
            //else parent_block = parent_block->parent;
        }
    }

    code_pos = 0;
}

/**
 *  Processes code buffer
 */
void larva_digest()
{
    var_t *token_var;
    char token[PARSER_MAX_TOKEN_SIZE];
    char oper[PARSER_MAX_TOKEN_SIZE];

    while (code[code_pos])
    {
        // find first significant character
        if (isspace(code[code_pos]))
        {
            ++code_pos;
            continue;
        }

        size_t line_start = code_pos;
        larva_read_token(token);

        // start with the most often operator
        if (!memcmp(token, "}\0", 2))
        {
            bc_add_cmd(BCO_BLOCK_END);
        }
        else if (!memcmp(token, "var\0", 4))
        {
            re_read_var:

            larva_read_token(token);

            if (   !memcmp(token, "if\0", 3)
                || !memcmp(token, "else\0", 5)
                || !memcmp(token, "var\0", 4)
                || !memcmp(token, "block\0", 6)
                || !memcmp(token, "return\0", 7)
                || !memcmp(token, "while\0", 6)
            )
            {
                fprintf(stderr, "Token '%s' is reserved and cannot be used as a variable name.", token);
                larva_error(0);
            }

            // skip spaces
            if (code[code_pos] == ' ') ++code_pos;

            // variable is just initialized, but not defined
            if (code[code_pos] == '\n')
            {
                bc_add_cmd(BCO_VAR);
                bc_add_token(token);
                continue;
            }

            if (code[code_pos] == ',')
            {
                ++code_pos;
                goto re_read_var;
            }

            if (code[code_pos] != '=')
            {
                fprintf(stderr, "Operator '=' expected, found '%c' '%s'", code[code_pos], token);
                larva_error(0);
            }

            ++code_pos;

            parse();

            bc_add_cmd(BCO_VARX);
            bc_add_token(token);

            // we have one more var to init
            if (code[code_pos] == ',')
            {
                ++code_pos;
                goto re_read_var;
            }
        }
        else if (!memcmp(token, "return\0", 7))
        {
            if (gl_level == 0)
            {
                if (verbose) puts("Returning from zero level. Bye-bye!");
                return;
            }

            // read space
            ++code_pos;

            parse();

            if (verbose) fprintf(stdout, "Returning from level %u...\n", gl_level);

            return;
        }
        else if (!memcmp(token, "block\0", 6))
        {
            larva_read_token(token);
            bc_add_cmd(BCO_BLOCK);
            bc_add_token(token);

            while (code[code_pos]) if (code[code_pos++] == '{') break;
        }
        else if (!memcmp(token, "if\0", 3))
        {
            unsigned long expr_start = code_pos, level = 0;
            // find expression
            while (code[code_pos])
            {
                if (code[code_pos] == '(') ++level;
                else if (code[code_pos] == ')')
                {
                    --level;
                    if (level < 1) { ++code_pos; break; }
                }
                ++code_pos;
            }

            size_t diff = code_pos - expr_start;
            char *expr = malloc(diff + 1);
            memcpy(expr, code + expr_start, diff + 1);
            expr[diff] = '\0';

            bc_add_cmd(BCO_IF);
            parse_expression(expr, diff);
            bc_add_cmd(BCO_CEIT);
            free(expr);
        }
        else if (!memcmp(token, "while\0", 6))
        {
            unsigned long expr_start = code_pos, level = 0;
            // find expression
            while (code[code_pos])
            {
                if (code[code_pos] == '(') ++level;
                else if (code[code_pos] == ')')
                {
                    --level;
                    if (level < 1) { ++code_pos; break; }
                }
                ++code_pos;
            }

            size_t diff = code_pos - expr_start;
            char *expr = malloc(diff + 1);
            memcpy(expr, code + expr_start, diff + 1);
            expr[diff] = '\0';

            bc_add_cmd(BCO_WHILE);
            parse_expression(expr, diff);
            bc_add_cmd(BCO_CEIT);
            free(expr);
        }
        else if (!memcmp(token, "else\0", 5))
        {
            /*if (run_flag[gl_level + 1] == 1) larva_skip_block();
            else
            {
                // find where else-block begins
                while (code[code_pos]) if (code[code_pos++] == '{') break;
                run_flag[++gl_level] = 1;
            }*/
            bc_add_cmd(BCO_ELSE);
        }
        else if (!memcmp(token, "{\0", 2))
        {
            bc_add_cmd(BCO_BLOCK_START);
        }
        else
        {
            code_pos = line_start;
            // result is not fed anywhere, free it
            parse();
        }
    }
}

void larva_read_token(char *token)
{
    while (isspace(code[code_pos])) ++code_pos;

    size_t start = code_pos, token_pos = 0;

    // read until newline
    while (++code_pos < code_length)
    {
        if (++token_pos == PARSER_MAX_TOKEN_SIZE)
        {
            larva_error("Token name is too long.");
        }

        // either it's a function call or just a command
        if (code[code_pos] == '(' || code[code_pos] == ',' || code[code_pos] == '=' || isspace(code[code_pos])) break;
    }

    memcpy(token, code + start, token_pos);

    token[token_pos] = '\0';
}

void larva_skip_block()
{
    unsigned int level = 0;
    while (code[code_pos])
    {
        if (code[code_pos] == '{') ++level;
        else if (code[code_pos] == '}' && --level < 1)
        {
            ++code_pos;
            return;
        }
        ++code_pos;
    }
}

void larva_error(char *err)
{
    unsigned int line = 1, sym = 0, i = 0;

    if (err) fputs(err, stderr);

    while (code[i] != '\0')
    {
        ++i;
        ++sym;
        if (code[i] == 10 || code[i] == 13)
        {
            ++line;
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
void larva_stop()
{
    if (verbose)
    {
        puts("================= DUMP ===============");
        larva_poo();
    //    puts("=============== BYTECODE =============");
    //    bc_poo();
        puts("======================================");
    }

    var_table_delete(vars);
    block_table_delete(blocks);

    if (code) free(code);

    bc_free();

#ifndef __APPLE__
    muntrace();
#endif
}

void larva_poo()
{
    unsigned int i;
    char types[][10] = {"UNSET", "DOUBLE", "STRING", "BLOCK", "ARRAY", "A-ARRAY", "PTR"};

    var v;
    var_t *v_list;
    block_t *b_list;

    for (i = 0; i < vars->size; ++i)
    {
        for (v_list = vars->table[i]; v_list != NULL; v_list = v_list->next)
        {
            fprintf(stdout, "'%s' [%s of size %u] = ", v_list->name, types[v_list->type], v_list->data_size);

            v = var_as_var_t(v_list);
            var_echo(&v);
            putc('\n', stdout);
        }
    }

    for (i = 0; i < blocks->size; ++i)
    {
        for (b_list = blocks->table[i]; b_list != NULL; b_list = b_list->next)
        {
            if (b_list->pos) fprintf(stdout, "Block '%s' at pos %u\n", b_list->name, b_list->pos);
        }
    }
}
