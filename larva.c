#include "common.h"
#include "larva.h"

char gl_errmsg[256];

/**
 *  Sets up the processor
 */
void larva_init()
{
    vars_count = MIN_VARIABLES;
    vars = calloc(MIN_VARIABLES, sizeof(var));
    for (unsigned long i = 0; i < MIN_VARIABLES; i++) vars[i].type = VAR_UNSET;
    gl_code = NULL;
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
}


/**
 *  Processes code buffer
 */
int larva_digest(char *code, size_t length)
{
    unsigned long index;
    size_t pos = 0;
    char token[PARSER_MAX_TOKEN_SIZE];
    char oper[PARSER_MAX_TOKEN_SIZE];

    // memorize
    gl_code = code;

    while (pos < length)
    {
        index = 0;

        if (code[pos] == '#')
        {
            if (pos < length - 1 && code[++pos] == '#')
            {
                // wait until '##' is met again
                while (pos++ < length) if (code[pos - 1] == '#' && code[pos] == '#') break;
            }
            else while (pos++ < length) if (code[pos] == 10 || code[pos] == 13) break;
            continue;
        }

        // find first significant character
        if (code[pos] == ';' || isspace(code[pos]))
        {
            // next please
            pos++;
            continue;
        }

        size_t line_start = pos;
        read_token(code, (void *)&pos, token);
        index = var_get_index(token);

        if (!strcmp(token, "var"))
        {
            read_token(code, (void *)&pos, token);

            if (index)
            {
                sprintf(gl_errmsg, "Variable '%s' already exists.", token);
                larva_error(pos);
            }

            // TODO: check that token is not in the list of reserved words

            // no var -- create it
            if (!index) index = var_init(token, VAR_STRING, 0);

            // variable is just initialized, but not defined
            if (code[pos] == ';') continue;
            
            // expect operator '='
            read_token(code, (void *)&pos, oper);

            if (strcmp(oper, "="))
            {
                sprintf(gl_errmsg, "Operator '=' expected, found '%s'", oper);
                larva_error(pos);
            }

            // equalize
            var_set_by_index(index, parse(&pos), 0);
        }
        else if (!strcmp(token, "if"))
        {
            unsigned long expr_start = pos, level = 0;
            // find expression
            while (code[pos])
            {
                if (code[pos] == '(') level++;
                if (code[pos] == ')')
                {
                    level--;
                    if (level < 1) { pos++; break; }
                }
                pos++;
            }

            char *expr = calloc(pos - expr_start, sizeof(char));
            memcpy(expr, &code[expr_start], pos - expr_start + 1);
            expr[pos - expr_start] = '\0';

            var x = parse_expression(expr);

            if (var_to_double(x))
            {
                fputs("true\n", stdout);
            }
            else fputs("false\n", stdout);

            free(expr);
        }
        else if (index)
        {
            pos = line_start;
            parse(&pos);
        }
        else
        {
            if (pos == length - 1) return larva_stop(0);

            strcpy(gl_errmsg, "Unknown token: ");
            strcat(gl_errmsg, token);
            larva_error(pos);
        }
    }

    return larva_stop(0);
}

void larva_error(unsigned long pos)
{
    int line = 1, sym = 0; size_t i = 0;
    while (gl_code[i] != '\0')
    {
        i++;
        sym++;
        if (gl_code[i] == 10 || gl_code[i] == 13)
        {
            line++;
            sym = 0;
        }

        if (i == pos) break;
    }

    fprintf(stderr, "\nError: %s on line %d, sym %d.\n\n", gl_errmsg, line, sym);

    larva_stop(ERR_SYNTAX);
}

size_t read_token(char *code, size_t *pos, char *token)
{
    size_t start = *pos;

    // read until newline
    while (code[(*pos)] != '\0')
    {
        token[(*pos) - start] = code[(*pos)];
        (*pos)++;
        // either it's a function call or just a command
        if (code[(*pos)] == '(' || code[(*pos)] == ';' || isspace(code[(*pos)])) break;
    }

    token[(*pos) - start] = '\0';
    trim(token);

    return (*pos) - start;
}

/**
 *  Call to stop execution
 */
int larva_stop(int code)
{
    fputs("=============== DUMP =============", stdout);
    larva_poo();
    fputs("\n==================================\n", stdout);

    unsigned int i = vars_count;
    while (--i) var_delete_by_index(i);

    if (vars) free(vars);
    if (gl_code) free(gl_code);

    exit(code);
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
        if (vars[i].type) fprintf(stdout, "\n'%s' [%s of size %lu] = ", vars[i].name, types[vars[i].type], vars[i].data_size);
        var_echo(vars[i]);
    }
}
