#include "var.h"

var var_assign(var a, var b)
{
    // 'a' will be overwritten in any case
    if (a.data) free(a.data);

    a.data = malloc(b.data_size);
    memcpy(a.data, b.data, b.data_size);

    a.type = b.type;
    a.data_size = b.data_size;

 /*   if (a.name)
    {
        unsigned int i = var_get_index(a.name);

        vars[i].type = a.type;
        vars[i].data_size = a.data_size;

        if (vars[i].data) free(vars[i].data);
        vars[i].data = malloc(a.data_size);
        memcpy(vars[i].data, a.data, a.data_size);
    }*/

    if (!b.name) free(b.data);

    return a;
}

var var_add(var a, var b)
{
    var r = a;

    if (a.type == VAR_STRING && b.type == VAR_STRING)
    {
        r.data = malloc(sizeof(char) * (a.data_size + b.data_size - 1));
        strcpy(r.data, a.data);
        strcat(r.data, b.data);
    }
    else if (a.type == VAR_DOUBLE && b.type == VAR_DOUBLE)
    {
        double xa, xb;
        memcpy(&xa, a.data, sizeof(double));
        memcpy(&xb, b.data, sizeof(double));

        xa += xb;

        r.data = malloc(sizeof(double));
        memcpy(r.data, &xa, sizeof(double));
    }
    else if (a.type == VAR_STRING && b.type == VAR_DOUBLE)
    {
        double xb;
        char *converted = calloc(256, sizeof(char));        // the length is doubtful
        memcpy(&xb, b.data, sizeof(double));
        sprintf(converted, "%.6g", xb);

        r.data = malloc(sizeof(char) * (a.data_size + strlen(converted) - 1));
        r.data = strcpy(r.data, a.data);
        r.data = strcat(r.data, converted);

        r.data_size = strlen(r.data);

        free(converted);
    }
    else if (a.type == VAR_DOUBLE && b.type == VAR_STRING)
    {
        double xa;
        char *converted = calloc(256, sizeof(char));        // the length is doubtful
        memcpy(&xa, a.data, sizeof(double));
        sprintf(converted, "%.6g", xa);

        r.data = malloc(sizeof(char) + (b.data_size + strlen(converted) - 1));
        strcpy(r.data, converted);
        strcat(r.data, b.data);

        r.type = VAR_STRING;
        r.data_size = strlen(r.data);

        free(converted);
    }
    else
    {
        fprintf(stderr, "Operator '+' is not defined for given operands.");
        larva_error();
    }


    // if there's no name it means that operations with that var are done
    // var itself is static, but its payload is dynamic, kill it with fire!
    if (!a.name) free(a.data);
    if (!b.name) free(b.data);

    return r;
}

var var_subtract(var a, var b)
{
    var r = a;

    if (a.type == VAR_DOUBLE && b.type == VAR_DOUBLE)
    {
        double xa, xb;
        memcpy(&xa, a.data, sizeof(double));
        memcpy(&xb, b.data, sizeof(double));
        if (!a.name) free(a.data);
        if (!b.name) free(b.data);

        xa = xa - xb;

        r.data = malloc(sizeof(double));
        memcpy(r.data, &xa, sizeof(double));
    }
    else
    {
        fprintf(stderr, "Operator '-' is not defined for given operands.");
        larva_error();
    }

    return r;
}

var var_multiply(var a, var b)
{
    var r = a;
    double xa, xb;
    memcpy(&xa, a.data, sizeof(double));
    memcpy(&xb, b.data, sizeof(double));
    if (!a.name) free(a.data);
    if (!b.name) free(b.data);

    xa *= xb;

    r.data = malloc(sizeof(double));
    memcpy(r.data, &xa, sizeof(double));

    return r;
}

var var_divide(var a, var b)
{
    var r = a;
    double xa, xb;
    memcpy(&xa, a.data, sizeof(double));
    memcpy(&xb, b.data, sizeof(double));
    if (!a.name) free(a.data);
    if (!b.name) free(b.data);

    xa /= xb;

    r.data = malloc(sizeof(double));
    memcpy(a.data, &xa, sizeof(double));

    return r;
}

var var_invert(var a)
{
    var r = a;

    if (a.type == VAR_DOUBLE)
    {
        double xa;
        memcpy(&xa, a.data, sizeof(double));
        if (!a.name) free(a.data);

        xa = -xa;

        r.data = malloc(sizeof(double));
        memcpy(r.data, &xa, sizeof(double));
    }
    else if (a.type == VAR_STRING)
    {
        // revert a string

        char *end = a.data + strlen(a.data) - 1;
        #define XOR_SWAP(a, b) do\
        {\
            a ^= b;\
            b ^= a;\
            a ^= b;\
        } while (0)

        // walk inwards from both ends of the string, 
        // swapping until we get to the middle
        while (a.data < end)
        {
            XOR_SWAP(*a.data, *end);
            a.data++;
            end--;
        }
        #undef XOR_SWAP
    }
    else
    {
        fprintf(stderr, "Inversion operator is not defined for the given operand.");
        larva_error();
    }

    return r;
}

var var_increment(var a)
{
    if (a.type == VAR_DOUBLE)
    {
        double xa;
        memcpy(&xa, a.data, sizeof(double));
        xa++;
        memcpy(a.data, &xa, sizeof(double));
    }
    else
    {
        fprintf(stderr, "Operator '++' is not defined for given operands.");
        larva_error();
    }

    return a;
}

var var_decrement(var a)
{
    if (a.type == VAR_DOUBLE)
    {
        double xa;
        memcpy(&xa, a.data, sizeof(double));
        xa--;
        memcpy(a.data, &xa, sizeof(double));
    }
    else
    {
        fprintf(stderr, "Operator '--' is not defined for given operands.");
        larva_error();
    }

    return a;
}

BYTE var_is_more(var a, var b)
{
    return (*a.data) > (*b.data);
}

BYTE var_is_less(var a, var b)
{
    return (*a.data) < (*b.data);
}

BYTE var_is_more_equal(var a, var b)
{
    return (*a.data) >= (*b.data);
}

BYTE var_is_less_equal(var a, var b)
{
    return (*a.data) <= (*b.data);
}

var var_array_element(var a, unsigned int i)
{
    // TODO
    a = var_as_double(42);
    return a;
}
