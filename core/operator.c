#include <math.h>
#include "var.h"
#include "var_2.h"
#include "map_2.h"
#include "array_2.h"
#include "sys.h"
#include "map.h"
#include "array.h"
#include "larva.h"
#include "bytecode.h"
#include "common.h"
#include "../vendor/jansson.h"

void op_assign(var *a, var *b)
{
    a->type = b->type;
    a->data = b->data;
    a->data_size = b->data_size;
}

void op_copy(bombyx_env_t *env, var *a, var *b)
{
    if (a->data)
    {
        if (b->data)
        {
            if (a->data_size == b->data_size && a->type == b->type)
            {
                if (a->type == VAR_DOUBLE) *(double *)a->data = *(double *)b->data;
                else if (a->type == VAR_MAP) a->data = map_table_clone(env, b->data);
                else if (a->type == VAR_ARRAY) a->data = array_clone(env, b->data);
                else if (a->type == VAR_POINTER) a->data = b->data;
                else memcpy(a->data, b->data, b->data_size);
                // it's done :)
                return;
            }
            else
            {
                if (a->type == VAR_DOUBLE) chfree(env->pool_of_doubles, a->data);
                else if (a->type == VAR_MAP) map_table_delete(env, a->data);
                else if (a->type == VAR_ARRAY) array_delete(env, a->data);
                else if (a->type != VAR_POINTER) free(a->data);

                if (b->type == VAR_MAP) a->data = map_table_clone(env, b->data);
                else if (b->type == VAR_ARRAY) a->data = array_clone(env, b->data);
                else if (b->type == VAR_POINTER) a->data = b->data;
                else
                {
                    a->data = malloc(b->data_size);
                    memcpy(a->data, b->data, b->data_size);
                }
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
                a->data = challoc(env->pool_of_doubles);
                *(double *)a->data = *(double *)b->data;
            }
            else if (b->type == VAR_MAP) a->data = map_table_clone(env, b->data);
            else if (b->type == VAR_ARRAY) a->data = array_clone(env, b->data);
            else if (b->type == VAR_POINTER) a->data = b->data;
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

void op_add(bombyx_env_t *env, var *a, var *b)
{
    var r = {0};

    if (a->type == VAR_STRING && b->type == VAR_STRING)
    {
        size_t t = --a->data_size;
        a->data_size = a->data_size + b->data_size;
        a->data = realloc(a->data, a->data_size);
        memcpy(a->data + t, b->data, b->data_size);
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
        var_unset(env, a);
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
        var_unset(env, a);
        a->data = malloc(r.data_size);
        memcpy(a->data, r.data, r.data_size);
        a->type = VAR_STRING;
        a->data_size = r.data_size;

        free(converted);
    }
    else
    {
        larva_error(env, "Operator '+' is not defined for given operands.");
    }

    if (r.data) free(r.data);
}

void op_subtract(bombyx_env_t *env, var *a, var *b)
{
    if (a->type == VAR_DOUBLE && b->type == VAR_DOUBLE)
    {
        *(double *)a->data -= *(double *)b->data;
    }
    else
    {
        larva_error(env, "Operator '-' is not defined for given operands.");
    }
}

void op_multiply(bombyx_env_t *env, var *a, var *b)
{
    if (a->type == VAR_DOUBLE && b->type == VAR_DOUBLE)
    {
        *(double *)a->data *= *(double *)b->data;
    }
    else
    {
        larva_error(env, "Operator '*' is not defined for given operands.");
    }
}

void op_divide(bombyx_env_t *env, var *a, var *b)
{
    if (a->type == VAR_DOUBLE && b->type == VAR_DOUBLE)
    {
        *(double *)a->data /= *(double *)b->data;
    }
    else
    {
        larva_error(env, "Operator '/' is not defined for given operands.");
    }
}

void op_power(bombyx_env_t *env, var *a, var *b)
{
    if (a->type == VAR_DOUBLE || b->type == VAR_DOUBLE)
    {
        *(double *)a->data = pow(*(double *)a->data, *(double *)b->data);
    }
    else
    {
        larva_error(env, "Operator '^' is not defined for the given operand type.");
    }
}

void op_modulo(bombyx_env_t *env, var *a, var *b)
{
    if (a->type == VAR_DOUBLE || b->type == VAR_DOUBLE)
    {
        *(double *)a->data = fmod(*(double *)a->data, *(double *)b->data);
    }
    else
    {
        larva_error(env, "Operator '%' is not defined for the given operand type.");
    }
}

void op_unary_minus(bombyx_env_t *env, var *a)
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
        larva_error(env, "Unary minus operator is not defined for the given operand.");
    }
}

/*
    Logical negation
*/
void op_invert(bombyx_env_t *env, var *a)
{
    if (a->type == VAR_DOUBLE)
    {
        *(double *)a->data = *(double *)a->data ? 0 : 1;
    }
    else
    {
        larva_error(env, "Inversion operator is not defined for the given operand.");
    }
}

void op_increment(bombyx_env_t *env, var *a)
{
    if (a->type == VAR_DOUBLE)
    {
        ++*(double *)a->data;
        if (a->ref) *(double *)a->ref->v.data = *(double *)a->data;
    }
    else
    {
        larva_error(env, "Operator '++' is not defined for the given operand type.");
    }
}

void op_decrement(bombyx_env_t *env, var *a)
{
    if (a->type == VAR_DOUBLE)
    {
        --*(double *)a->data;
        if (a->ref) *(double *)a->ref->v.data = *(double *)a->data;
    }
    else
    {
        larva_error(env, "Operator '--' is not defined for the given operand type.");
    }
}

void op_swap(var *a, var *b)
{
    // TODO: don't swap name and ref
    var t = *a;
    *a = *b;
    *b = t;
}

void op_and(bombyx_env_t *env, var *a, var *b)
{
    // TODO: implement
}

void op_or(bombyx_env_t *env, var *a, var *b)
{
    // TODO: implement
}

BYTE var_is_true(bombyx_env_t *env, var *a)
{
    if (a->type == VAR_DOUBLE) return *(double *)a->data != 0;
    else if (a->type == VAR_STRING) return a->data_size > 1;
    else
    {
        larva_error(env, "Comparison operator is not defined for the given operand type.");
    }
    return 0;
}

/*
    '1' is 'equal'
*/
BYTE var_cmp(bombyx_env_t *env, var *a, var *b)
{
    if (a->type == VAR_DOUBLE && b->type == VAR_DOUBLE) return memcmp(a->data, b->data, sizeof(double)) ? 0 : 1;
    else if (a->type == VAR_STRING && b->type == VAR_STRING) return strcmp(a->data, b->data) ? 0 : 1;
    else
    {
        larva_error(env, "Comparison operator is not defined for the given operand type.");
    }
    return 0;
}

BYTE var_is_more(bombyx_env_t *env, var *a, var *b)
{
    // TODO: discuss string more/less comparisons
    if (a->type == VAR_DOUBLE && b->type == VAR_DOUBLE) return *(double *)a->data > *(double *)b->data;
    else if (a->type == VAR_STRING && b->type == VAR_STRING) return strcmp(a->data, b->data) > 0;
    else
    {
        larva_error(env, "Operator '>' is not defined for the given operand type.");
    }
    return 0;
}

BYTE var_is_less(bombyx_env_t *env, var *a, var *b)
{
    // TODO: discuss string more/less comparisons
    if (a->type == VAR_DOUBLE && b->type == VAR_DOUBLE) return *(double *)a->data < *(double *)b->data;
    else if (a->type == VAR_STRING && b->type == VAR_STRING) return strcmp(a->data, b->data) < 0;
    else
    {
        larva_error(env, "Operator '<' is not defined for the given operand type.");
    }
    return 0;
}

BYTE var_is_more_equal(bombyx_env_t *env, var *a, var *b)
{
    if (a->type == VAR_DOUBLE && b->type == VAR_DOUBLE) return *(double *)a->data >= *(double *)b->data;
    else
    {
        larva_error(env, "Operator '>=' is defined for numbers only.");
    }
    return 0;
}

BYTE var_is_less_equal(bombyx_env_t *env, var *a, var *b)
{
    if (a->type == VAR_DOUBLE && b->type == VAR_DOUBLE) return *(double *)a->data <= *(double *)b->data;
    else
    {
        larva_error(env, "Operator '<=' is defined for numbers only.");
    }
    return 0;
}
