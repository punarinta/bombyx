#include "var.h"
#include "sys.h"
#include "larva.h"
#include "bytecode.h"
#include "../common.h"
#include "../vendor/jansson.h"

void op_copy(var *a, var *b)
{
    if (a->data)
    {
        if (b->data)
        {
            if (a->data_size == b->data_size && a->type == b->type)
            {
                if (a->type == VAR_DOUBLE) *(double *)a->data = *(double *)b->data;
                else memcpy(a->data, b->data, b->data_size);
                // it's done :)
                return;
            }
            else
            {
                if (a->type == VAR_DOUBLE) chfree(pool_of_doubles, a->data);
                else free(a->data);

                a->data = malloc(b->data_size);
                memcpy(a->data, b->data, b->data_size);
            }
        }
        else
        {
            a->data = NULL;
        }
    }
    else
    {
        // 'a' is empty
        if (b->data)
        {
            if (b->type == VAR_DOUBLE)
            {
                a->data = challoc(pool_of_doubles);
                *(double *)a->data = *(double *)b->data;
            }
            else
            {
                a->data = malloc(b->data_size);
                memcpy(a->data, b->data, b->data_size);
            }
        }
    }

    a->data_size = b->data_size;
    a->type = b->type;
    a->ref = b->ref;
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

        var_unset(a);
        a->data = malloc(r.data_size);
        strcpy(a->data, r.data);
        a->data_size = r.data_size;
    }
    else if (a->type == VAR_DOUBLE && b->type == VAR_DOUBLE)
    {
        *(double *)a->data += *(double *)b->data;
    }
    else if (a->type == VAR_STRING && b->type == VAR_DOUBLE)
    {
        double xb;
        char *converted = malloc(256);
        memcpy(&xb, b->data, sizeof(double));
        unsigned int len = sprintf(converted, "%.6g", xb);

        r.data_size = a->data_size + len;
        r.data = malloc(r.data_size);
        memcpy(r.data, a->data, a->data_size - 1);
        memcpy(r.data + a->data_size - 1, converted, len);

        *((char *) (r.data + r.data_size - 1)) = '\0';

        // realloc
        var_unset(a);
        a->data = malloc(r.data_size);
        memcpy(a->data, r.data, r.data_size);
        a->data_size = r.data_size;

        free(converted);
    }
    else if (a->type == VAR_DOUBLE && b->type == VAR_STRING)
    {
        double xa;
        char *converted = malloc(256);
        memcpy(&xa, a->data, sizeof(double));
        unsigned int len = sprintf(converted, "%.6g", xa);

        r.data_size = len + b->data_size;
        r.data = malloc(r.data_size);
        memcpy(r.data, converted, len);
        memcpy(r.data + len, b->data, b->data_size);

        // realloc
        var_unset(a);
        a->data = malloc(r.data_size);
        memcpy(a->data, r.data, r.data_size);
        a->type = VAR_STRING;
        a->data_size = r.data_size;

        free(converted);
    }
    else
    {
        larva_error("Operator '+' is not defined for given operands.");
    }

    if (r.data) free(r.data);
}

void op_subtract(var *a, var *b)
{
    if (a->type == VAR_DOUBLE && b->type == VAR_DOUBLE)
    {
        *(double *)a->data -= *(double *)b->data;
    }
    else
    {
        larva_error("Operator '-' is not defined for given operands.");
    }
}

void op_multiply(var *a, var *b)
{
    if (a->type == VAR_DOUBLE && b->type == VAR_DOUBLE)
    {
        *(double *)a->data *= *(double *)b->data;
    }
    else
    {
        larva_error("Operator '*' is not defined for given operands.");
    }
}

void op_divide(var *a, var *b)
{
    if (a->type == VAR_DOUBLE && b->type == VAR_DOUBLE)
    {
        *(double *)a->data /= *(double *)b->data;
    }
    else
    {
        larva_error("Operator '/' is not defined for given operands.");
    }
}

void op_unary_minus(var *a)
{
    if (a->type == VAR_DOUBLE)
    {
        *(double *)a->data = -*(double *)a->data;
    }
    else if (a->type == VAR_STRING)
    {
        // revert a string
        char *ptr = a->data;
        char *end = a->data + a->data_size - 2;

        #define XOR_SWAP(a, b) do\
        {\
            a ^= b;\
            b ^= a;\
            a ^= b;\
        } while (0)

        // walk inwards from both ends of the string, swapping until we get to the middle
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
        larva_error("Unary minus operator is not defined for the given operand.");
    }
}

/*
    Logical negation
*/
void op_invert(var *a)
{
    if (a->type == VAR_DOUBLE)
    {
        *(double *)a->data = *(double *)a->data ? 0 : 1;
    }
    else
    {
        larva_error("Inversion operator is not defined for the given operand.");
    }
}

void op_increment(var *a)
{
    if (a->type == VAR_DOUBLE)
    {
        ++*(double *)a->data;
    }
    else
    {
        larva_error("Operator '++' is not defined for the given operand type.");
    }

    if (a->ref) op_copy(&((var_t *)a->ref)->v, a);
}

void op_decrement(var *a)
{
    if (a->type == VAR_DOUBLE)
    {
        --*(double *)a->data;
    }
    else
    {
        larva_error("Operator '--' is not defined for the given operand type.");
    }

    if (a->ref) op_copy(&((var_t *)a->ref)->v, a);
}

void op_swap(var *a, var *b)
{
    var t = *a;
    *a = *b;
    *b = t;
}

void op_and(var *a, var *b)
{
    // TODO: implement
}

void op_or(var *a, var *b)
{
    // TODO: implement
}

BYTE var_is_true(var *a)
{
    if (a->type == VAR_DOUBLE) return *(double *)a->data != 0;
    else if (a->type == VAR_STRING) return a->data_size > 1;
    else
    {
        larva_error("Comparison operator is not defined for the given operand type.");
    }
    return 0;
}

/*
    '1' is 'equal'
*/
BYTE var_cmp(var *a, var *b)
{
    if (a->type == VAR_DOUBLE && b->type == VAR_DOUBLE) return memcmp(a->data, b->data, sizeof(double)) ? 0 : 1;
    else if (a->type == VAR_STRING && b->type == VAR_STRING) return strcmp(a->data, b->data);
    else
    {
        larva_error("Comparison operator is not defined for the given operand type.");
    }
    return 0;
}

BYTE var_is_more(var *a, var *b)
{
    if (a->type == VAR_DOUBLE && b->type == VAR_DOUBLE) return *(double *)a->data > *(double *)b->data;
    else if (a->type == VAR_STRING && b->type == VAR_STRING) return strcmp(a->data, b->data) > 0;
    else
    {
        larva_error("Operator '>' is not defined for the given operand type.");
    }
    return 0;
}

BYTE var_is_less(var *a, var *b)
{
    if (a->type == VAR_DOUBLE && b->type == VAR_DOUBLE) return *(double *)a->data < *(double *)b->data;
    else if (a->type == VAR_STRING && b->type == VAR_STRING) return strcmp(a->data, b->data) < 0;
    else
    {
        larva_error("Operator '<' is not defined for the given operand type.");
    }
    return 0;
}

BYTE var_is_more_equal(var *a, var *b)
{
    if (a->type == VAR_DOUBLE && b->type == VAR_DOUBLE) return *(double *)a->data >= *(double *)b->data;
    else
    {
        larva_error("Operator '>=' is defined for numbers only.");
    }
    return 0;
}

BYTE var_is_less_equal(var *a, var *b)
{
    if (a->type == VAR_DOUBLE && b->type == VAR_DOUBLE) return *(double *)a->data <= *(double *)b->data;
    else
    {
        larva_error("Operator '<=' is defined for numbers only.");
    }
    return 0;
}
