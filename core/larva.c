#include "common.h"
#include "larva.h"
#include "var.h"
#include "var_2.h"
#include "cocoon_2.h"
#include "block.h"
#include "sys.h"
#include "bytecode.h"
#include "expression.h"
#include "debugger.h"

/**
 *  Sets up the processor
 */
void larva_init(bombyx_env_t *env, char *incoming_code, size_t len)
{
#ifndef BOMBYX_WEB
    env->blocks             = block_table_create(MIN_BLOCKS);
    env->vars               = var_table_create(MIN_VARIABLES);
    env->cocoons            = cocoon_table_create(MIN_COCOONS);
    env->pool_of_doubles    = chcreate(POOL_OF_DOUBLES_SIZE, sizeof(double));
#endif

    bc_init(env);

    env->code_pos = 0;
    char token[PARSER_MAX_TOKEN_SIZE];

    // strip da shit out
    env->code = larva_chew(env, incoming_code, len, &env->code_length);

    while (env->code_pos < env->code_length)
    {
        size_t line_start = env->code_pos;

        if (line_start && env->code[line_start - 1] != 13 && env->code[line_start - 1] != 10)
        {
            // just skip such cases for now
            ++env->code_pos;
        }

        larva_read_token(env, token);

        if (!memcmp(token, "include\0", 8))
        {
            larva_read_string_token(env, token);

#ifdef BOMBYX_WEB
            static pthread_mutex_t include_mutex = PTHREAD_MUTEX_INITIALIZER;
            pthread_mutex_lock(&accept_mutex);
#endif

            FILE *fp = fopen(token, "rt");
            if (!fp)
            {
                fprintf(stderr, "Cannot include file '%s'.", token);
                larva_error(env, 0);
            }

            fseek(fp, 0L, SEEK_END);
            long bufsize = ftell(fp);
            char *source = malloc(bufsize + 1);
            fseek(fp, 0L, SEEK_SET);
            size_t file_len = fread(source, sizeof(char), bufsize, fp);

            // included files do not contain trailing zeroes

            size_t included_size;
            source = larva_chew(env, source, file_len, &included_size);

            // now 'source' contains cleaned code from included file
            // we insert this code into code_pos and increase code_length on its length

            env->code = realloc(env->code, env->code_length + included_size - (env->code_pos - line_start) + 1);

            if (!env->code)
            {
                larva_error(env, "Your system has been hijacked by alohasnackbars.");
            }

            // move the unprocessed code
            // thus the trailing zero will be moved to the very end
            memmove(env->code + line_start + 1 + included_size, env->code + env->code_pos, env->code_length - env->code_pos);

            // move the included source
            memmove(env->code + line_start + 1, source, included_size);

            // insert a newline
            env->code[line_start] = '\n';

            env->code_length += (included_size - (env->code_pos - line_start));

            // temporary
            free(source);

            fclose(fp);

#ifdef BOMBYX_WEB
            pthread_mutex_unlock(&accept_mutex);
#endif
        }
        else if (!memcmp(token, "use\0", 4))
        {
            larva_read_token(env, token);
            cocoon_t *cocoon = cocoon_add(env, env->cocoons, token);
        }

        ++env->code_pos;
    }
}

char *larva_chew(bombyx_env_t *env, char *incoming_code, size_t len, size_t *new_len)
{
    BYTE quotes_on = 0;
    char *new_code = malloc(len + 1);

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
void larva_digest(bombyx_env_t *env)
{
    BYTE is_param;
    BYTE expect_block = 0;
    env->code_pos = 0;
    char token[PARSER_MAX_TOKEN_SIZE];

    while (env->code[env->code_pos])
    {
        size_t line_start = env->code_pos;
        larva_read_token(env, token);

        // start with the most often operator
        if (!memcmp(token, "}\0", 2))
        {
            bc_add_cmd(env, BCO_BLOCK_END);
        }
        else if (!memcmp(token, "var\0", 4))
        {
            is_param = 0;

            re_read_var:

            larva_read_token(env, token);

            if (   !memcmp(token, "if\0", 3)
                || !memcmp(token, "else\0", 5)
                || !memcmp(token, "var\0", 4)
                || !memcmp(token, "block\0", 6)
                || !memcmp(token, "return\0", 7)
                || !memcmp(token, "while\0", 6)
            )
            {
                fprintf(stderr, "Token '%s' is reserved and cannot be used as a variable name.", token);
                larva_error(env, 0);
            }

            // skip spaces
            while (env->code[env->code_pos] == ' ') ++env->code_pos;

            // variable is just initialized, but not defined
            if (env->code[env->code_pos] == '\n' || !env->code[env->code_pos])
            {
                bc_add_cmd(env, is_param ? BCO_PARAM : BCO_VAR);
                bc_add_token(env, token);
                continue;
            }

            if (env->code[env->code_pos] == ',')
            {
                bc_add_cmd(env, is_param ? BCO_PARAM : BCO_VAR);
                bc_add_token(env, token);
                ++env->code_pos;
                goto re_read_var;
            }

            if (env->code[env->code_pos] == ')')
            {
                bc_add_cmd(env, BCO_PARAM);
                bc_add_token(env, token);
                ++env->code_pos;
                continue;
            }

            if (env->code[env->code_pos] != '=')
            {
                fprintf(stderr, "Operator '=' expected, found '%c' '%s'", env->code[env->code_pos], token);
                larva_error(env, 0);
            }

            ++env->code_pos;

            // skip spaces
            while (env->code[env->code_pos] == ' ') ++env->code_pos;

            if (env->code[env->code_pos] == '{' || env->code[env->code_pos] == '[')
            {
                // This is madness!
                // Madness? No. This. Is. JSON!

                size_t json_string_start = env->code_pos;
                while (env->code[env->code_pos] && env->code[env->code_pos] != '\n' && env->code[env->code_pos] != '\r' && env->code[env->code_pos] != ')')
                {
                    ++env->code_pos;
                }

                char *json_string = malloc(env->code_pos - json_string_start + 1);
                memcpy(json_string, env->code + json_string_start, env->code_pos - json_string_start);
                json_string[env->code_pos - json_string_start] = 0;

                bc_add_cmd(env, BCO_FROM_JSON);
                bc_add_string(env, json_string);

                free(json_string);

                bc_add_cmd(env, BCO_VARX);
                bc_add_token(env, token);
            }
            else
            {
                parse(env);

                bc_add_cmd(env, is_param ? BCO_PARAMX : BCO_VARX);
                bc_add_token(env, token);
            }

            // we have one more var to init
            if (env->code[env->code_pos] == ',')
            {
                ++env->code_pos;
                goto re_read_var;
            }
        }
        else if (!memcmp(token, "return\0", 7))
        {
            bc_add_cmd(env, BCO_CLEAR_STACK);

            // parse until newline
            parse(env);
            bc_add_cmd(env, BCO_RETURN);
        }
        else if (!memcmp(token, "block\0", 6))
        {
            larva_read_token(env, token);
            bc_add_cmd(env, BCO_BLOCK_DEF);
            bc_add_token(env, token);
            bc_add_cmd(env, BCO_BLOCK_START);
            expect_block = 1;

            if (env->code[env->code_pos] == '(')
            {
                ++env->code_pos;
                is_param = 1;
                goto re_read_var;
            }
        }
        else if (!memcmp(token, "if\0", 3))
        {
            unsigned long expr_start = env->code_pos, level = 0;

            // find expression
            while (env->code[env->code_pos])
            {
                if (env->code[env->code_pos] == '(') ++level;
                else if (env->code[env->code_pos] == ')')
                {
                    --level;
                    if (level < 1) { ++env->code_pos; break; }
                }
                ++env->code_pos;
            }

            size_t diff = env->code_pos - expr_start;
            char *expr = malloc(diff + 1);
            memcpy(expr, env->code + expr_start, diff + 1);
            expr[diff] = '\0';

            bc_add_cmd(env, BCO_IF);
            parse_expression(env, expr, diff);
            bc_add_cmd(env, BCO_CEIT);
            free(expr);
        }
        else if (!memcmp(token, "while\0", 6))
        {
            unsigned long expr_start = env->code_pos, level = 0;

            // find expression
            while (env->code[env->code_pos])
            {
                if (env->code[env->code_pos] == '(') ++level;
                else if (env->code[env->code_pos] == ')')
                {
                    --level;
                    if (level < 1) { ++env->code_pos; break; }
                }
                ++env->code_pos;
            }

            size_t diff = env->code_pos - expr_start;
            char *expr = malloc(diff + 1);
            memcpy(expr, env->code + expr_start, diff + 1);
            expr[diff] = '\0';

            bc_add_cmd(env, BCO_WHILE);
            parse_expression(env, expr, diff);
            bc_add_cmd(env, BCO_CEIT);
            free(expr);
        }
        else if (!memcmp(token, "else\0", 5))
        {
            bc_add_cmd(env, BCO_ELSE);
        }
        else if (!memcmp(token, "{\0", 2))
        {
            if (!expect_block) bc_add_cmd(env, BCO_BLOCK_START);
            else expect_block = 0;
        }
        else if (!memcmp(token, "use\0", 4))
        {
            // TODO(?): cut out uses within chew()

            // just skip
            larva_read_token(env, token);
        }
        else
        {
            env->code_pos = line_start;
            parse(env);
        }

        ++env->code_pos;
    }
}

void larva_read_token(bombyx_env_t *env, char *token)
{
    while (isspace(env->code[env->code_pos])) ++env->code_pos;

    size_t token_pos = 0, start = env->code_pos;

    // read until newline
    while (++env->code_pos < env->code_length)
    {
        if (++token_pos == PARSER_MAX_TOKEN_SIZE)
        {
            larva_error(env, "Token name is too long.");
        }

        // either it's a function call or just a command
        if (env->code[env->code_pos] == '('
        || env->code[env->code_pos] == ')'
        || env->code[env->code_pos] == ','
        || env->code[env->code_pos] == '='
        || isspace(env->code[env->code_pos])
        || env->code[env->code_pos] == '{'
        || env->code[env->code_pos] == '}' ) break;
    }

    memcpy(token, env->code + start, token_pos);

    token[token_pos] = '\0';
}

void larva_read_string_token(bombyx_env_t *env, char *token)
{
    while (isspace(env->code[env->code_pos])) ++env->code_pos;

    if (env->code[env->code_pos] != '\'') larva_error(env, "String token should be placed into quotes.");

    size_t token_pos = 0, start = ++env->code_pos;

    // read until newline
    while (++env->code_pos < env->code_length)
    {
        if (++token_pos == PARSER_MAX_TOKEN_SIZE) larva_error(env, "Token name is too long.");
        if (env->code[env->code_pos] == '\'') break;
        if (env->code[env->code_pos] == 13) larva_error(env, "Multiline string tokens are not allowed");
    }
    ++env->code_pos;

    memcpy(token, env->code + start, token_pos);

    token[token_pos] = '\0';
}

void larva_skip_block(bombyx_env_t *env)
{
    unsigned int level = 0;
    while (env->code[env->code_pos])
    {
        if (env->code[env->code_pos] == '{') ++level;
        else if (env->code[env->code_pos] == '}' && --level < 1)
        {
            ++env->code_pos;
            return;
        }
        ++env->code_pos;
    }
}

void larva_error(bombyx_env_t *env, char *err, ...)
{
    unsigned int line = 1, sym = 0, i = 0;

    if (err)
    {
        char error_text[256];
        va_list args;
        va_start(args, err);
        vsprintf(error_text, err, args);
        web_puts(env, error_text);
        va_end(args);
    }

    while (env->code[i] != '\0')
    {
        ++i;
        ++sym;
        if (env->code[i] == 10 || env->code[i] == 13)
        {
            ++line;
            sym = 0;
        }

        if (i == env->code_pos) break;
    }

    web_printf(env, "\nError on line %d, sym %d.\n\n", line, sym);
    printf("BC pos = %d\n", env->bc_pos);

    env->gl_error = 1;
    longjmp(env->error_exit, 1);
}

/**
 *  Call to stop execution
 */
void larva_stop(bombyx_env_t *env)
{
    if (verbose)
    {
        puts("================= DUMP ===============");
#ifdef BOMBYX_WEB
        puts("BOMBYX_WEB = 1");
#endif
        larva_poo(env);
        stack_poo(env);
#ifdef BOMBYX_DEBUG
        debug_env(env);
#endif
    }

#ifdef BOMBYX_WEB
    var_table_delete(env, env->vars, 1);
    block_table_delete(env->blocks, 1);
    cocoon_table_delete(env->cocoons, 1);
    chclear(&env->pool_of_doubles);
#else
    var_table_delete(env, env->vars, 0);
    block_table_delete(env->blocks, 0);
    cocoon_table_delete(env->cocoons, 0);
    chdestroy(&env->pool_of_doubles);
#endif

    if (env->code) free(env->code);

    bc_free(env);

#ifdef BOMBYX_MCHECK
    muntrace();
#endif
}

void larva_poo(bombyx_env_t *env)
{
    unsigned int i;
    char types[][10] = {"UNSET", "DOUBLE", "STRING", "-reserved-", "BLOCK", "ARRAY", "MAP", "CUSTOM"};

    var v;
    var_t *v_list;
    block_t *b_list;

    for (i = 0; i < env->vars->size; ++i)
    {
        for (v_list = env->vars->table[i]; v_list != NULL; v_list = v_list->next)
        {
            fprintf(stdout, "'%s' [%s of size %u] = ", v_list->v.name, types[v_list->v.type], v_list->v.data_size);

            var_echo(env, &v_list->v);
            putc('\n', stdout);
        }
    }

    for (i = 0; i < env->blocks->size; ++i)
    {
        for (b_list = env->blocks->table[i]; b_list != NULL; b_list = b_list->next)
        {
            if (b_list->pos) fprintf(stdout, "Block '%s' at pos %u\n", b_list->name, b_list->pos);
        }
    }
}
