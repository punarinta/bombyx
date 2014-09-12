#include "common.h"
#include "larva.h"

char *command = NULL, *object = NULL, *oper = NULL; // 'operator' can be reserved
char gl_errmsg[256];

size_t read_command(char *, size_t *);
void larva_poo();

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
    unsigned long index = 0;
    size_t pos = 0;

    // memorize
    gl_code = code;

    while (pos < length)
    {
        index = 0;

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

        // find first significant character
        if (code[pos] == ';' || isspace(code[pos]))
        {
            // next please
            pos++;
            continue;
        }

        // collect garbage
        if (command) free(command);


        size_t command_start = pos;
        size_t command_size = read_command(code, (void *)&pos);
        command = malloc(sizeof(char) * (command_size + 1));
        memcpy(command, &code[command_start], command_size);
        command[command_size] = 0;

        trim(command);

        index = var_get_index(command);

        if (!strcmp(command, "var"))
        {
            // skip possible spaces
            read_until_not_token(code, (void *)&pos, ' ');

            size_t object_start = pos;
            size_t object_size = read_until_token(code, (void *)&pos, ' ');
            if (object) free(object);
            object = malloc(sizeof(char) * (object_size + 1));
            memcpy(object, (void *)&code[object_start], object_size);
            object[object_size] = 0;

            trim(object);

            if (var_get_index(object))
            {
                sprintf(gl_errmsg, "Variable '%s' already exists.", object);
                larva_error(pos);
            }

            // variable is just initialized, but not defined
            if (code[pos] == ';')
            {
                if (!var_get_index(object))
                {
                    fprintf(stdout, "Creating variable '%s'\n", object);
                    var_init(object, VAR_STRING, 0);
                }
                continue;
            }
            
            // expect operator '='
            size_t oper_start = pos;
            size_t oper_size = read_until_token(code, (void *)&pos, ' ');
            if (oper) free(oper);
            oper = malloc(sizeof(char) * (oper_size + 1));
            memcpy(oper, (void *)&code[oper_start], oper_size);
            oper[oper_size] = 0;
            trim(oper);

            if (strcmp(oper, "="))
            {
                sprintf(gl_errmsg, "Operator '=' expected, found '%s'", oper);
                larva_error(pos);
            }

        /*    size_t expression_start = pos;
            size_t expression_size = read_until_token(code, (void *)&pos, ';');
            if (expression) free(expression);
            expression = malloc(sizeof(char) * (expression_size + 1));
            memcpy(expression, (void *)&code[expression_start], expression_size);
            expression[expression_size] = 0;
            trim(expression);*/

            if (index)
            {
            //    fprintf(stdout, "Assigning variable '%s' to '%s'\n", object, expression);
                var_set_by_index(index, parse(&pos), 0);
            }
            else
            {
            //    fprintf(stdout, "Creating variable '%s' as '%s'\n", object, expression);
                var_set_by_index(var_init(object, VAR_STRING, 0), parse(&pos), 0);
            }
        }
        else if (index)
        {
            // this is an object, find an operator
            
            if (code[pos] == '(')
            {
                // check if this is an internal function
                if (0)
                {
                    continue;
                }

            /*    size_t expression_start = pos;
                size_t expression_size = read_until_token(code, (void *)&pos, ';');
                if (expression) free(expression);
                expression = malloc(sizeof(char) * (expression_size + 1));
                memcpy(expression, (void *)&code[expression_start], expression_size);
            
                expression[expression_size] = 0;
                fprintf(stdout, "Parsing expression '%s' and executing function '%s' with it\n", expression, command);

                pos = command_start;*/


                // this will parse the function call as a part of an expression

                pos = command_start;
                parse(&pos);

                continue;
            }

            size_t oper_start = pos;
            size_t oper_size = read_until_token(code, (void *)&pos, ' ');
            if (oper) free(oper);
            oper = malloc(sizeof(char) * (oper_size + 1));
            memcpy(oper, (void *)&code[oper_start], oper_size);
            oper[oper_size] = 0;
            trim(oper);

            if (!strlen(oper))
            {
                // dummy
                // TODO: add warnings
                continue;
            }
            
            if (!strcmp(oper, "="))
            {
            /*    size_t expression_start = pos;
                size_t expression_size = read_until_token(code, (void *)&pos, ';');
                if (expression) free(expression);
                expression = malloc(sizeof(char) * (expression_size + 1));
                memcpy(expression, (void *)&code[expression_start], expression_size);
                expression[expression_size] = 0;
                trim(expression);

                fprintf(stdout, "Parsing expression '%s'\n", expression);*/

                var_set_by_index(index, parse(&pos), 0);
            }
            else
            {
                sprintf(gl_errmsg, "Unknown operator '%s'", oper);
                larva_error(pos);
            }
        }
        else
        {
            if (pos == length - 1) return larva_stop(0);

            strcpy(gl_errmsg, "Unknown token");
            larva_error(pos);
        }
    }

    return larva_stop(0);
}

void larva_error(unsigned long pos)
{
    int line = 1, sym = 0; size_t i = 0;
    while (gl_code[i] != 0)
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

    fprintf(stderr, "\nError: %s\nLast command is '%s' (line %d, sym %d).\n\n", gl_errmsg, command, line, sym);

    larva_stop(ERR_SYNTAX);
}

size_t read_command(char *code, size_t *pos)
{
    size_t start = *pos;
    size_t length = strlen(code);

    // read until newline
    while ((*pos) < length)
    {
        (*pos)++;
        // either it's a function call or just a command
        if (code[(*pos)] == '(' || code[(*pos)] == ';' || isspace(code[(*pos)])) break;
    }

    return (*pos) - start;
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
 *  Call to stop execution
 */
int larva_stop(int code)
{
    fputs("=============== DUMP =============", stdout);
    larva_poo();
    fputs("\n==================================\n", stdout);

    unsigned long i = vars_count;
    while (--i) var_delete_by_index(i);

    if (vars) free(vars);
    if (gl_code) free(gl_code);
    if (command) free(command);
    if (object) free(object);
  //  if (expression) free(expression);
    if (oper) free(oper);

    exit(code);
}

void larva_poo()
{
    char types[20][16];

    strcpy(types[VAR_UNSET],    "UNSET");
    strcpy(types[VAR_BYTE],     "BYTE");
    strcpy(types[VAR_WORD],     "WORD");
    strcpy(types[VAR_DWORD],    "DWORD");
    strcpy(types[VAR_QWORD],    "QWORD");
    strcpy(types[VAR_FLOAT],    "FLOAT");
    strcpy(types[VAR_STRING],   "STRING");

    for (unsigned long i = 1; i < vars_count; i++)
    {
        if (vars[i].type) fprintf(stdout, "\n'%s' [%s of size %lu] = ", vars[i].name, types[vars[i].type], vars[i].data_size);

        switch (vars[i].type)
        {
            case VAR_STRING:
            fprintf(stdout, "%s", vars[i].data);
            break;

            case VAR_BYTE:
            fprintf(stdout, "%d", vars[i].data[0]);
            break;

            case VAR_WORD:
            fprintf(stdout, "%d", (unsigned) (vars[i].data[0] + 256 * vars[i].data[1]));
            break;

            case VAR_DWORD:
            fprintf(stdout, "%lu", (unsigned long) (vars[i].data[0] + 256 * vars[i].data[1] + 65536 * vars[i].data[2] + 16777216 * vars[i].data[3]));
            break;

            default:
            break;
        }
    }
}
