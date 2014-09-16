#include "common.h"
#include "larva.h"


/**
 *  Sets up the processor
 */
void larva_init(char *incoming_code, unsigned int len)
{
    vars_count = MIN_VARIABLES;
    vars = calloc(MIN_VARIABLES, sizeof(var));
    for (unsigned long i = 0; i < MIN_VARIABLES; i++) vars[i].type = VAR_UNSET;
    code = incoming_code;
    code_pos = 0;
    code_length = len;

    started_at = get_microtime();

    // TODO: cut all the commented code
    // TODO: include all the necessary files
}


/**
 *  Allocates more space for variables
 */
void larva_grow(unsigned long size)
{
    if (!size)
    {
        // by default just double it
        size = MIN_VARIABLES;
    }

    vars = realloc(vars, sizeof(var) * (vars_count + size));

    if (!vars) larva_stop(ERR_NO_MEMORY);
    vars_count += size;
}


/**
 *  Processes code buffer
 */
int larva_digest()
{
    unsigned int index;
    char token[PARSER_MAX_TOKEN_SIZE];
    char oper[PARSER_MAX_TOKEN_SIZE];
    BYTE run_next_block;
    gl_error = 0;

    setjmp(error_exit);
    if (gl_error)
    {
        return larva_stop(gl_error);
    }

    while (code[code_pos])
    {
        index = 0;
        run_next_block = 0;

        if (code[code_pos] == '#')
        {
            if (code_pos < code_length - 1 && code[++code_pos] == '#')
            {
                // wait until '##' is met again
                while (code_pos++ < code_length) if (code[code_pos - 1] == '#' && code[code_pos] == '#') break;
            }
            else while (code_pos++ < code_length) if (code[code_pos] == '\n') break;
            continue;
        }

        // find first significant character
        if (isspace(code[code_pos]))
        {
            // next please
            code_pos++;
            continue;
        }

        size_t line_start = code_pos;
        read_token(token);
        index = var_get_index(token);

        if (!strcmp(token, "var"))
        {
            read_token(token);

            if (index)
            {
                fprintf(stderr, "Variable '%s' already exists.", token);
                larva_error(code_pos);
            }

            // TODO: check that token is not in the list of reserved words

            // no var -- create it
            if (!index) index = var_init(token, VAR_STRING, 0);

            // variable is just initialized, but not defined
            if (code[code_pos] == '\n') continue;
            
            // expect operator '='
            read_token(oper);

            if (strcmp(oper, "="))
            {
                fprintf(stderr, "Operator '=' expected, found '%s'", oper);
                larva_error(code_pos);
            }

            // equalize
            var_set_by_index(index, parse(&code_pos), 0);
        }
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

            char *expr = calloc(code_pos - expr_start, sizeof(char));
            memcpy(expr, &code[expr_start], code_pos - expr_start + 1);
            expr[code_pos - expr_start] = '\0';

            var x = parse_expression(expr);

            if (var_to_double(x))
            {
                run_next_block = 1;
            }
            else
            {
                // skip the whole block
                skip_block();
            }

            free(expr);
        }
        else
        {
            code_pos = line_start;
            parse(&code_pos);
        }
    }

    return larva_stop(0);
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
        if (code[code_pos] == '(' || isspace(code[code_pos])) break;
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
        fputs("=============== DUMP =============", stdout);
        larva_poo();
        fputs("\n==================================\n", stdout);
    }

    unsigned int i = vars_count;
    while (--i) var_delete_by_index(i);

    if (vars) free(vars);

#ifndef __APPLE__
    muntrace();
#endif

    return ret_code;
}

void larva_poo()
{
    char types[20][16];

    // TODO: get rid of this stupid copying :)
    strcpy(types[VAR_UNSET],    "UNSET");
    strcpy(types[VAR_BYTE],     "BYTE");
    strcpy(types[VAR_WORD],     "WORD");
    strcpy(types[VAR_DWORD],    "DWORD");
    strcpy(types[VAR_QWORD],    "QWORD");
    strcpy(types[VAR_FLOAT],    "FLOAT");
    strcpy(types[VAR_DOUBLE],   "DOUBLE");
    strcpy(types[VAR_STRING],   "STRING");

    for (unsigned int i = 1; i < vars_count; i++)
    {
        if (vars[i].type) fprintf(stdout, "\n'%s' [%s of size %u] = ", vars[i].name, types[vars[i].type], vars[i].data_size);
        var_echo(vars[i]);
    }
}
