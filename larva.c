#include "larva.h"

/**
 *    Sets up the processor
 */
void larva_init()
{
    vars_count = MIN_VARIABLES;
    vars = malloc(sizeof(var) * MIN_VARIABLES);

    var_add("foo", VAR_GENERIC, "qwwew");

    fputs(vars[0].name, stderr);
}

/**
 *    Processes code buffer
 */
int larva_digest(char *code, size_t length)
{
    size_t pos = 0;
    char token[32];

    while (pos < length)
    {
        // find first significant character
        if (code[pos] == ' ' || code[pos] == 10 || code[pos] == 13)
        {
            // next please
            pos++;
            continue;
        }

        if (code[pos] == '#')
        {
            read_until_newline(code, (void *)&pos);
            continue;
        }

        // memorize token start position
        int token_start = pos;
        int token_size = 0;

        while (pos < length)
        {
            token_size = pos - token_start;
            if (token_size >= 32)
            {
                return larva_stop(-1);
            }

            if (code[pos] == ' ')
            {
                token[token_size] = '\0';
                break;
            }

            token[token_size] = code[pos];

            pos++;
        }

        if (!strcmp(token, "var"))
        {
            read_until_newline(code, (void *)&pos);
        }
        else if (!strcmp(token, "varx"))
        {
            read_until_newline(code, (void *)&pos);
        }
        else
        {
            // TODO: count line number and report it

            fputs("Syntax error. Last token is '", stderr);
            fputs(token, stderr);
            fputs("'.\n", stderr);

            return larva_stop(-1);
        }

   /*     fputs(token, stdout);
        fputs("\n", stdout);

        pos++;*/
    }


 /*   var a;
    var *b = malloc(sizeof(var) * 10);
    free(b);
    fputs(code, stdout);*/

    return 0;
}

/**
 *    Reads until new line is not met
 */
void read_until_newline(char *code, size_t *pos)
{
    size_t length = strlen(code);

    // read until newline
    while ((*pos) < length)
    {
        (*pos)++;
        if (code[(*pos)] == 10 || code[(*pos)] == 13)
        {
            break;
        }
    }
}


/**
 *    Call to stop execution
 */
int larva_stop(int code)
{
    if (vars)
    {
        free(vars);
    }

    return code;
}
