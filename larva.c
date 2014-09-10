#include "larva.h"

/**
 *  Sets up the processor
 */
void larva_init()
{
    vars_count = MIN_VARIABLES;
    vars = malloc(sizeof(var) * MIN_VARIABLES);

    // skip first variable
    var_add(NULL, VAR_UNSET, NULL);
}

/**
 *  Processes code buffer
 */
int larva_digest(char *code, size_t length)
{
    size_t pos = 0;
    char *operator = NULL, *operand = NULL, *expression = NULL;

    while (pos < length)
    {
        // find first significant character
        if (code[pos] == ';' || isspace(code[pos]))
        {
            // next please
            pos++;
            continue;
        }

        if (code[pos] == '#')
        {
            // read until newline
            while (pos < length)
            {
                pos++;
                if (code[pos] == 10 || code[pos] == 13) break;
            }
            continue;
        }

        // collect garbage
        if (operator) free(operator);
        if (operand) free(operand);
        if (expression) free(expression);

        size_t operator_start = pos;
        size_t operator_size = read_until_token(code, (void *)&pos, ' ');
        operator = malloc(sizeof(char) * (operator_size + 1));
        memcpy(operator, &code[operator_start], operator_size);
        operator[operator_size] = 0;

        if (!strcmp(operator, "var"))
        {
            // skip possible spaces
            read_until_not_token(code, (void *)&pos, ' ');

            size_t operand_start = pos;
            size_t operand_size = read_until_token(code, (void *)&pos, '=');
            operand = malloc(sizeof(char) * operand_size);
            memcpy(operand, (void *)&code[operand_start], operand_size);
            strcpy(operand, trim(operand));

            // if does not exist -- create
            if (!var_get_index(operand))
            {
                fprintf(stdout, "Creating variable '%s'\n", operand);
                var_add(operand, VAR_GENERIC, 0);
            }

            if (code[pos] == ';') continue;

            size_t expression_start = pos;
            size_t expression_size = read_until_token(code, (void *)&pos, ';');
            expression = malloc(sizeof(char) * expression_size);
            memcpy(expression, (void *)&code[expression_start], expression_size);

            fprintf(stdout, "Assigning variable '%s' to '%s'\n", operand, expression);
        }
        else
        {
            // no more operators
            if (pos == length - 1) return 0;

            int line = 1, sym = 0;
            for (size_t i = 0; i < length; i++)
            {
                sym++;
                if (code[i] == 10 || code[i] == 13)
                {
                    line++;
                    sym = 0;
                }

                if (i == pos) break;
            }

            fprintf(stderr, "Line %d, position %d: last token is '%s'.\n", line, sym, operator);

            return larva_stop(-1);
        }
    }

    return 0;
}

/**
 *  Reads until token character is met or the expression is not closed
 */
size_t read_until_token(char *code, size_t *pos, char token)
{
    size_t start = *pos;
    size_t length = strlen(code);

    // read until newline
    while ((*pos) < length)
    {
        (*pos)++;
        if (code[(*pos)] == token || code[(*pos)] == ';') break;
    }

    return (*pos) - start;
}

/**
 *  Reads until token character stops meeting
 */
size_t read_until_not_token(char *code, size_t *pos, char token)
{
    size_t start = *pos;
    size_t length = strlen(code);

    // read until newline
    while ((*pos) < length)
    {
        (*pos)++;
        if (code[(*pos)] != token) break;
    }

    return (*pos) - start;
}


/**
 *    Call to stop execution
 */
int larva_stop(int code)
{
    if (vars) free(vars);

    return code;
}

char *trim(char *str)
{
    char *end;
    while (isspace(*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;
    *(end+1) = 0;

    return str;
}