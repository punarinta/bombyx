#include "../common.h"
#include "larva.h"
#include "var.h"
#include "block.h"
#include "sys.h"
#include "bytecode.h"

/**
 *  Sets up the processor
 */
void larva_init(char *incoming_code, size_t len)
{
    blocks_count = MIN_BLOCKS;
    vars_count   = MIN_VARIABLES;
    blocks       = block_table_create(MIN_BLOCKS);
    vars         = var_table_create(MIN_VARIABLES);

    bc_init();

    code_pos = 0;
    char token[PARSER_MAX_TOKEN_SIZE];

    // strip da shit out
    code = larva_chew(incoming_code, len, &code_length);

    while (code[code_pos])
    {
        size_t line_start = code_pos;

        larva_read_token(token);

        if (!memcmp(token, "include\0", 8))
        {
            larva_read_string_token(token);

            FILE *fp = fopen(token, "rt");
            if (!fp)
            {
                fprintf(stderr, "Cannot include file '%s'.", token);
                larva_error(0);
            }

            fseek(fp, 0L, SEEK_END);
            long bufsize = ftell(fp);
            char *source = malloc(bufsize + 1);
            fseek(fp, 0L, SEEK_SET);
            size_t file_len = fread(source, sizeof(char), bufsize, fp);

            // included files do not contain trailing zeroes

            size_t included_size;
            source = larva_chew(source, file_len, &included_size);

            // now 'source' contains cleaned code from included file
            // we insert this code into code_pos and increase code_length on its length

            code = realloc(code, code_length + included_size - (code_pos - line_start));

            if (!code)
            {
                larva_error("Your system has been hijacked by alohasnackbars.");
            }

            // move the unprocessed code
            // thus the trailing zero will be moved to the very end
            memmove(code + line_start + included_size, code + code_pos, code_length - code_pos);

            // move the included source
            memmove(code + line_start, source, included_size);

            code_length += (included_size - (code_pos - line_start));

            // temporary
            free(source);

            fclose(fp);
        }
        else if (!memcmp(token, "use\0", 4))
        {
            larva_read_token(token);

            int x;
            char *error;
            void *lib_handle;
            double (*fn)(int *);

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

        ++code_pos;
    }
}

char *larva_chew(char *incoming_code, size_t len, size_t *new_len)
{
    BYTE quotes_on = 0;
    char *new_code = malloc(len);

    *new_len = 0;

    for (unsigned int i = 0; i < len; ++i)
    {
        if (incoming_code[i] == '\'') quotes_on = !quotes_on;

        if (i < len - 1 && incoming_code[i] == '\\')
        {
            if (incoming_code[i + 1] == 'n') new_code[*new_len] = '\n';
            else if (incoming_code[i + 1] == 't') new_code[*new_len] = '\t';
            else --(*new_len);
            ++i;
        }
        else if (incoming_code[i] == '#' && !quotes_on)
        {
            if (i < len - 1 && incoming_code[i + 1] == '#')
            {
                while (i++ < len) if (incoming_code[i] == '#' && incoming_code[i + 1] == '#') { ++i; break; }
            }
            else while (i++ < len) if (incoming_code[i] == '\n') break;

            continue;
        }
        else
        {
            new_code[*new_len] = incoming_code[i];
        }

        ++(*new_len);
    }

    free(incoming_code);

    return new_code;
}

/**
 *  Processes code buffer
 */
void larva_digest()
{
    code_pos = 0;
    char token[PARSER_MAX_TOKEN_SIZE];

    while (code[code_pos])
    {
        size_t line_start = code_pos;
        larva_read_token(token);

        // start with the most often operator
        if (!memcmp(token, "}\0", 2))
        {
            bc_add_cmd(BCO_BLOCK_END);
        }
        else if (!memcmp(token, "var\0", 4) || !memcmp(token, "param\0", 6))
        {
            BYTE is_param = 0;
            if (!memcmp(token, "param\0", 6)) is_param = 1;

            re_read_var:

            larva_read_token(token);

            if (   !memcmp(token, "if\0", 3)
                || !memcmp(token, "else\0", 5)
                || !memcmp(token, "var\0", 4)
                || !memcmp(token, "block\0", 6)
                || !memcmp(token, "return\0", 7)
                || !memcmp(token, "while\0", 6)
                || !memcmp(token, "param\0", 6)
            )
            {
                fprintf(stderr, "Token '%s' is reserved and cannot be used as a variable name.", token);
                larva_error(0);
            }

            // skip spaces
            while (code[code_pos] == ' ') ++code_pos;

            // variable is just initialized, but not defined
            if (code[code_pos] == '\n' || !code[code_pos])
            {
                bc_add_cmd(is_param ? BCO_PARAM : BCO_VAR);
                bc_add_token(token);
                continue;
            }

            if (code[code_pos] == ',')
            {
                bc_add_cmd(is_param ? BCO_PARAM : BCO_VAR);
                bc_add_token(token);
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

            bc_add_cmd(is_param ? BCO_PARAMX : BCO_VARX);
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
            bc_add_cmd(BCO_CLEAR_STACK);

            // parse until newline
            parse();
            bc_add_cmd(BCO_RETURN);
        }
        else if (!memcmp(token, "block\0", 6))
        {
            larva_read_token(token);
            bc_add_cmd(BCO_BLOCK_DEF);
            bc_add_token(token);
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
            bc_add_cmd(BCO_ELSE);
        }
        else if (!memcmp(token, "{\0", 2))
        {
            bc_add_cmd(BCO_BLOCK_START);
        }
        else if (!memcmp(token, "use\0", 4))
        {
            // TODO(?): cut out uses within chew()

            // just skip
            larva_read_token(token);
        }
        else
        {
            code_pos = line_start;
            parse();
        }

        ++code_pos;
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

void larva_read_string_token(char *token)
{
    while (isspace(code[code_pos])) ++code_pos;

    if (code[code_pos] != '\'')
    {
        larva_error("String token should be placed into quotes.");
    }

    size_t start = ++code_pos, token_pos = 0;

    // read until newline
    while (++code_pos < code_length)
    {
        if (++token_pos == PARSER_MAX_TOKEN_SIZE)
        {
            larva_error("Token name is too long.");
        }

        if (code[code_pos] == '\'') break;
        if (code[code_pos] == 13)
        {
            larva_error("Multiline string tokens are not allowed");
        }
    }
    ++code_pos;

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
        puts("=============== BYTECODE =============");
        printf("Ops count = %u.\n", bc_ops);
        printf("Code size = %u byte(s).\n", bc_length);
        printf("Stack size = %u.\n", bc_stack_size);
    //    bc_poo();
        puts("======================================");
    }

    var_table_delete(vars);
    block_table_delete(blocks);

    if (code) free(code);

    bc_free();

#ifdef BOMBYX_MCHECK
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
            fprintf(stdout, "'%s' [%s of size %u] = ", v_list->v.name, types[v_list->v.type], v_list->v.data_size);

            var_echo(&v_list->v);
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
