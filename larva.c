#include "common.h"
#include "larva.h"
#include "core/var.h"
#include "core/block.h"
#include "core/sys.h"

BYTE keywcmp(char *a, char *b, size_t n)
{
    while (n --> 0)
    {
        if (a[n] != b[n]) return 1;
    }
    return 0;
}

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
    BYTE transfer_space = 0;

    // TODO: include all the necessary files

    for (unsigned int i = 0; i < len; ++i)
    {
        if (incoming_code[i] == '\'') quotes_on = !quotes_on;

        if (!quotes_on && (incoming_code[i] == ' ' || incoming_code[i] == '\t'))
        {
            if ((code_length >= 3 && !keywcmp(&code[code_length - 3], "var", 3))
            ||  (code_length >= 5 && !keywcmp(&code[code_length - 5], "block", 5))
            ||  (code_length >= 6 && !keywcmp(&code[code_length - 6], "return", 6))
            )
            {
                code[code_length] = incoming_code[i];
            }
            else continue;
        }
        else if (i < len - 1 && code_length && code[code_length - 1] == '\n' && incoming_code[i] == '\n')
        {
            // skip multiple newlines
            continue;
        }
        else if (i < len - 1 && incoming_code[i] == '\\')
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

    started_at = get_microtime();

    larva_chew();
}

void larva_chew()
{
    int not_allowed = 0;
    block_t *parent_block = NULL;
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
                larva_error(0);
            }

            // scan for '{'
            while (code[code_pos]) if (code[code_pos++] == '{') break;

            parent_block = block_add(blocks, token, code_pos, parent_block);
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
        else if (!strcmp(token, "if") || !strcmp(token, "else") )
        {
            ++not_allowed;
            while (code[code_pos]) if (code[code_pos++] == '{') break;
        }
        else if (!strcmp(token, "}"))
        {
            if (not_allowed) --not_allowed;
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

        if (!strcmp(token, "var"))
        {
            re_read_var:

            larva_read_token(token);

            if (   !strcmp(token, "if")
                || !strcmp(token, "else")
                || !strcmp(token, "var")
                || !strcmp(token, "block")
                || !strcmp(token, "return")
            )
            {
                fprintf(stderr, "Token '%s' is reserved and cannot be used as a variable name.", token);
                larva_error(0);
            }

            // should be no var -- create it
            token_var = var_add(vars, token, VAR_STRING, NULL);

            if (!token_var)
            {
                fprintf(stderr, "Variable '%s' already exists.", token);
                larva_error(0);
            }

            // variable is just initialized, but not defined
            if (code[code_pos] == '\n') continue;

            if (code[code_pos] == ',')
            {
                ++code_pos;
                goto re_read_var;
            }

            if (code[code_pos] != '=')
            {
                fprintf(stderr, "Operator '=' expected, found '%c'", code[code_pos]);
                larva_error(0);
            }

            ++code_pos;

            // equalize
            var *parse_result = parse();

            if (!parse_result)
            {
                larva_error("Operator '=' expects an value to follow");
            }

            if (parse_result->name) free(parse_result->name);
            parse_result->name = strdup(token);
            var_sync(parse_result);
            var_free(parse_result);

            // we have one more var to init
            if (code[code_pos] == ',')
            {
                ++code_pos;
                goto re_read_var;
            }
        }
        else if (!strcmp(token, "return"))
        {
            if (gl_level == 0)
            {
                if (verbose) puts("Returning from zero level. Bye-bye!");
                return NULL;
            }

            // read space
            ++code_pos;

            var *r = parse();

            // this var CANNOT have a name
            if (r->name)
            {
                free(r->name);
                r->name = NULL;
            }

            if (verbose) fprintf(stdout, "Returning from level %u...\n", gl_level);

            return r;
        }
        else if (!strcmp(token, "block")) larva_skip_block();
        else if (!strcmp(token, "if"))
        {
            unsigned long expr_start = code_pos, level = 0;
            // find expression
            while (code[code_pos])
            {
                if (code[code_pos] == '(') ++level;
                if (code[code_pos] == ')')
                {
                    --level;
                    if (level < 1) { ++code_pos; break; }
                }
                ++code_pos;
            }

            size_t diff = code_pos - expr_start;
            char *expr = malloc(diff + 1);
            memcpy(expr, &code[expr_start], diff + 1);
            expr[diff] = '\0';

            var *x = parse_expression(expr, diff);
            free(expr);

            // compare and unset 'x'
            if (!var_to_double(x))
            {
                run_flag[gl_level + 1] = 2;  // RUN_ELSE
                larva_skip_block();
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
            larva_error("Blocks should be named or preceeded by control statements.");
        }
        else if (!strcmp(token, "}"))
        {
            // if you met a bracket and 'run_flag' on this level is zero, then this is a function end
            if (!run_flag[gl_level])
            {
                return NULL;
            }
            else --gl_level;
        }
        else if (!strcmp(token, "else"))
        {
            if (run_flag[gl_level + 1] == 1) larva_skip_block();
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

void larva_read_token(char *token)
{
    size_t start = code_pos, token_pos = 0;

    // read until newline
    while (code[code_pos] != '\0')
    {
        if (token_pos == PARSER_MAX_TOKEN_SIZE)
        {
            larva_error("Token name is too long.");
        }

        if (!token_pos && code[code_pos] == ' ')
        {
            ++code_pos;
            continue;
        }

        token[token_pos] = code[code_pos];
        ++token_pos;
        ++code_pos;

        // either it's a function call or just a command
        if (code[code_pos] == '(' || code[code_pos] == ',' || code[code_pos] == '=' || isspace(code[code_pos])) break;
    }

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
        puts("======================================");
    }

    var_table_delete(vars);
    block_table_delete(blocks);

    if (code) free(code);

#ifndef __APPLE__
    muntrace();
#endif
}

void larva_poo()
{
    unsigned int i;
    char types[][10] = {"UNSET", "DOUBLE", "STRING", "BLOCK", "ARRAY", "A-ARRAY", "PTR"};

    var *v;
    var_t *v_list;
    block_t *b_list;

    for (i = 0; i < vars->size; ++i)
    {
        for (v_list = vars->table[i]; v_list != NULL; v_list = v_list->next)
        {
            fprintf(stdout, "'%s' [%s of size %u] = ", v_list->name, types[v_list->type], v_list->data_size);

            v = var_as_var_t(v_list);
            var_echo(v);
            var_free(v);
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
