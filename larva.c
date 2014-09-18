#include "common.h"
#include "larva.h"


/**
 *  Sets up the processor
 */
void larva_init(char *incoming_code, unsigned int len)
{
    vars_count   = MIN_VARIABLES;
    blocks_count = MIN_BLOCKS;
    vars   = calloc(MIN_VARIABLES, sizeof(var));
    blocks = calloc(MIN_BLOCKS, sizeof(block));

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

int larva_digest_start()
{
    gl_error = 0;
    gl_level = 0;
    gl_save_names = 0;

    setjmp(error_exit);

    if (gl_error) return larva_stop(gl_error);

    // no vars need to leave to the outer world
    var r = larva_digest();
    var_free(r);

    return larva_stop(0);
}

/**
 *  Processes code buffer
 */
var larva_digest()
{
    unsigned int index;
    char token[PARSER_MAX_TOKEN_SIZE];
    char oper[PARSER_MAX_TOKEN_SIZE];
    var r;

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
        index = var_get_index(token);

        if (!strcmp(token, "var"))
        {
            re_read_var:

            read_token(token);

            // check what's the status of this var
            index = var_get_index(token);

            if (index)
            {
                fprintf(stderr, "Variable '%s' already exists.", token);
                larva_error(code_pos);
            }

            // TODO: check that token is not in the list of reserved words

            // should be no var -- create it
            index = var_init(token, VAR_STRING, 0);

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
            var_set_by_index(index, parse());

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
                fprintf(stdout, "zero level\n");
                return vars[0];
            }

            r = parse();

            // this var CANNOT have a name
            if (r.name) { free(r.name); r.name = NULL; }

            gl_level--;
            code_pos = ret_point[gl_level];

            if (verbose) fprintf(stdout, "returning...\n");
            return r;
        }
        else if (!strcmp(token, "block"))
        {
            read_token(token);

            // check what's the status of this var
            index = block_get_index(token);

            if (index)
            {
                fprintf(stderr, "Block '%s' already exists.", token);
                larva_error(code_pos);
            }

            unsigned int startpos = code_pos;

            // move to the first symbol after '{'
            while (code[startpos])
            {
                if (code[startpos++] == '{') break;
            }

            index = block_init(startpos, token);

            skip_block();
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

            char *expr = calloc(code_pos - expr_start + 1, sizeof(char));
            memcpy(expr, &code[expr_start], code_pos - expr_start + 1);
            expr[code_pos - expr_start] = '\0';

            var x = parse_expression(expr);
            free(expr);

            if (!var_extract_double(x))
            {
                run_flag[gl_level] = 1;  // RUN_ELSE
                skip_block();
            }
            else
            {
                // start running this block
                run_flag[gl_level] = 0;  // RUN_NONE
            }
            x.name = NULL;
            var_free(x);
        }
        else if (!strcmp(token, "{"))
        {
            gl_level++;
        }
        else if (!strcmp(token, "}"))
        {
            gl_level--;
            if (!run_flag[gl_level] && ret_point[gl_level]) code_pos = ret_point[gl_level];
        }
        else if (!strcmp(token, "else"))
        {
            if (run_flag[gl_level] == 0) skip_block();
        }
        else
        {
            code_pos = line_start;
            // result is not fed anywhere, free it
            var_free(parse());
        }
    }

    return vars[0];
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

    unsigned int i = vars_count;
    while (--i) var_delete_by_index(i);

    i = blocks_count;
    while (--i) block_delete_by_index(i);

    if (vars) free(vars);
    if (blocks) free(blocks);
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

    for (i = 1; i < vars_count; i++) if (vars[i].type) 
    {
        fprintf(stdout, "\n'%s' [%s of size %u] = ", vars[i].name, types[vars[i].type], vars[i].data_size);
        var_echo(vars[i]);
    }
    for (i = 1; i < blocks_count; i++)
    {
        if (blocks[i].pos) fprintf(stdout, "\nBlock '%s' at pos %u", blocks[i].name, blocks[i].pos);
    }
}
