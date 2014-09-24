#include "var.h"
#include "sys.h"
#include "../common.h"

/*
    Synchronizes var with vars[] if necessary
    Var is NOT modified
*/
void var_sync(var *a)
{
    var_t *v = var_lookup(vars, a->name);
    if (v)
    {
        v->type = a->type;
        v->data_size = a->data_size;

        if (v->data)
        {
            free(v->data);
        }

        v->data = malloc(a->data_size);
        memcpy(v->data, a->data, a->data_size);
    }
}

void op_copy(var *a, var *b)
{
    if (a->data)
    {
        free(a->data);

        if (b->data)
        {
            a->data = malloc(b->data_size);
            memcpy(a->data, b->data, b->data_size);
        }
        else
        {
            a->data = NULL;
        }
    }

    a->data_size = b->data_size;
    a->type = b->type;
}

void op_add(var *a, var *b)
{
    var r = {0};

    if (a->type == VAR_STRING && b->type == VAR_STRING)
    {
        r.data_size = a->data_size + b->data_size - 1;
        r.data = malloc(r.data_size);
        memcpy(r.data, a->data, a->data_size - 1);
        memcpy(r.data + a->data_size - 1, b->data, b->data_size);   // copy together with EOS

        free(a->data);
        a->data = malloc(r.data_size);
        strcpy(a->data, r.data);
        a->data_size = r.data_size;
    }
    else if (a->type == VAR_DOUBLE && b->type == VAR_DOUBLE)
    {
        double xa, xb;
        memcpy(&xa, a->data, sizeof(double));
        memcpy(&xb, b->data, sizeof(double));

        xa += xb;

        memcpy(a->data, &xa, sizeof(double));
    }
    else if (a->type == VAR_STRING && b->type == VAR_DOUBLE)
    {
        double xb;
        char *converted = calloc(256, sizeof(char));
        memcpy(&xb, b->data, sizeof(double));
        sprintf(converted, "%.6g", xb);
        unsigned int len = strlen(converted);

        r.data_size = a->data_size + len;
        r.data = malloc(r.data_size);
        memcpy(r.data, a->data, a->data_size - 1);
        memcpy(r.data + a->data_size - 1, converted, len);

        r.data[r.data_size - 1] = '\0';

        // realloc
        free(a->data);
        a->data = malloc(r.data_size);
        strcpy(a->data, r.data);
        a->data_size = r.data_size;

        free(converted);
    }
    else if (a->type == VAR_DOUBLE && b->type == VAR_STRING)
    {
        double xa;
        char *converted = calloc(256, sizeof(char));
        memcpy(&xa, a->data, sizeof(double));
        sprintf(converted, "%.6g", xa);
        unsigned int len = strlen(converted);

        r.data_size = len + b->data_size;
        r.data = malloc(r.data_size);
        memcpy(r.data, converted, len);
        memcpy(r.data + len, b->data, b->data_size);

        // realloc
        free(a->data);
        a->data = malloc(r.data_size);
        strcpy(a->data, r.data);
        a->type = VAR_STRING;
        a->data_size = r.data_size;

        free(converted);
    }
    else
    {
        fprintf(stderr, "Operator '+' is not defined for given operands.");
        larva_error();
    }

    if (r.data) free(r.data);
}

void op_subtract(var *a, var *b)
{
    if (a->type == VAR_DOUBLE && b->type == VAR_DOUBLE)
    {
        double xa, xb;
        memcpy(&xa, a->data, sizeof(double));
        memcpy(&xb, b->data, sizeof(double));

        xa -= xb;

        memcpy(a->data, &xa, sizeof(double));
    }
    else
    {
        fprintf(stderr, "Operator '-' is not defined for given operands.");
        larva_error();
    }
}

void op_multiply(var *a, var *b)
{
    if (a->type == VAR_DOUBLE && b->type == VAR_DOUBLE)
    {
        double xa, xb;
        memcpy(&xa, a->data, sizeof(double));
        memcpy(&xb, b->data, sizeof(double));

        xa *= xb;

        memcpy(a->data, &xa, sizeof(double));
    }
    else
    {
        fprintf(stderr, "Operator '-' is not defined for given operands.");
        larva_error();
    }
}

void op_divide(var *a, var *b)
{
    if (a->type == VAR_DOUBLE && b->type == VAR_DOUBLE)
    {
        double xa, xb;
        memcpy(&xa, a->data, sizeof(double));
        memcpy(&xb, b->data, sizeof(double));

        xa /= xb;

        memcpy(a->data, &xa, sizeof(double));
    }
    else
    {
        fprintf(stderr, "Operator '-' is not defined for given operands.");
        larva_error();
    }
}

/*
    Inverts the var.
*/
void op_invert(var *a)
{
    if (a->type == VAR_DOUBLE)
    {
        double xa;
        memcpy(&xa, a->data, sizeof(double));

        xa = -xa;

        memcpy(a->data, &xa, sizeof(double));
    }
    else if (a->type == VAR_STRING)
    {
        // revert a string
        char *ptr = a->data;
        char *end = a->data + strlen(a->data) - 1;

        #define XOR_SWAP(a, b) do\
        {\
            a ^= b;\
            b ^= a;\
            a ^= b;\
        } while (0)

        // walk inwards from both ends of the string, 
        // swapping until we get to the middle
        while (ptr < end)
        {
            XOR_SWAP(*ptr, *end);
            ++ptr;
            --end;
        }
        #undef XOR_SWAP
    }
    else
    {
        fprintf(stderr, "Inversion operator is not defined for the given operand.");
        larva_error();
    }
}

/*
    Increments the variable
    Returns its value back
*/
void op_increment(var *a)
{
    if (!a->name)
    {
        fprintf(stderr, "Operator '++' requires a variable.");
        larva_error();
    }

    if (a->type == VAR_DOUBLE)
    {
        double xa;
        memcpy(&xa, a->data, sizeof(double));
        ++xa;
        memcpy(a->data, &xa, sizeof(double));
    }
    else
    {
        fprintf(stderr, "Operator '++' is not defined for the given operand type.");
        larva_error();
    }

    var_sync(a);
}

void op_decrement(var *a)
{
    if (!a->name)
    {
        fprintf(stderr, "Operator '--' requires a variable.");
        larva_error();
    }

    if (a->type == VAR_DOUBLE)
    {
        double xa;
        memcpy(&xa, a->data, sizeof(double));
        --xa;
        memcpy(a->data, &xa, sizeof(double));
    }
    else
    {
        fprintf(stderr, "Operator '--' is not defined for the given operand type.");
        larva_error();
    }

    var_sync(a);
}

BYTE var_is_more(var *a, var *b)
{
    if (a->type == VAR_DOUBLE && b->type == VAR_DOUBLE) return (*a->data) > (*b->data);
    else
    {
        fprintf(stderr, "Operator '>' is defined for numbers only.");
        larva_error();
    }
    return 0;
}

BYTE var_is_less(var *a, var *b)
{
    if (a->type == VAR_DOUBLE && b->type == VAR_DOUBLE) return (*a->data) < (*b->data);
    else
    {
        fprintf(stderr, "Operator '<' is defined for numbers only.");
        larva_error();
    }
    return 0;
}

BYTE var_is_more_equal(var *a, var *b)
{
    if (a->type == VAR_DOUBLE && b->type == VAR_DOUBLE) return (*a->data) >= (*b->data);
    else
    {
        fprintf(stderr, "Operator '>=' is defined for numbers only.");
        larva_error();
    }
    return 0;
}

BYTE var_is_less_equal(var *a, var *b)
{
    if (a->type == VAR_DOUBLE && b->type == VAR_DOUBLE) return (*a->data) <= (*b->data);
    else
    {
        fprintf(stderr, "Operator '<=' is defined for numbers only.");
        larva_error();
    }
    return 0;
}

/*
var *var_array_element(var *a, unsigned int i)
{
    var *a = NULL;

    return a;
}
*/
