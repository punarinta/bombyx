#include "var.h"

var var_assign(var a, var b)
{
    // 'a' will be overwritten in any case
    if (a.data) free(a.data);

    a.data = b.data;
    a.type = b.type;
    a.data_size = b.data_size;

    if (a.name) vars[var_get_index(a.name)] = a;

    return a;
}

var var_add(var a, var b)
{
    var r = a;

    if (a.type == VAR_STRING && b.type == VAR_STRING)
    {
        r.data = calloc(1, sizeof(char));
        strcat(r.data, a.data);
        strcat(r.data, b.data);
    }
    if (a.type == VAR_DOUBLE && b.type == VAR_DOUBLE)
    {
        double xa, xb;
        memcpy(&xa, a.data, sizeof(double));
        memcpy(&xb, b.data, sizeof(double));

        xa += xb;

        r.data = malloc(sizeof(double));
        memcpy(r.data, &xa, sizeof(double));
    }
    if (a.type == VAR_STRING && b.type == VAR_DOUBLE)
    {
        r.data = calloc(1, sizeof(char));
        strcat(r.data, a.data);

        double xb;
        char *converted = calloc(256, sizeof(char));        // the length is doubtful
        memcpy(&xb, b.data, sizeof(double));
        sprintf(converted, "%.6g", xb);
        strcat(r.data, converted);

        free(converted);
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
    double xa, xb;
    memcpy(&xa, a.data, sizeof(double));
    memcpy(&xb, b.data, sizeof(double));
    if (!a.name) free(a.data);
    if (!b.name) free(b.data);

    xa -= xb;

    r.data = calloc(1, sizeof(double));
    memcpy(r.data, &xa, sizeof(double));

    return a;
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

    r.data = calloc(1, sizeof(double));
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

    r.data = calloc(1, sizeof(double));
    memcpy(a.data, &xa, sizeof(double));

    return a;
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
    if (a.type == VAR_STRING)
    {
        // revert a string

        char *end = a.data + strlen(a.data) - 1;
        #define XOR_SWAP(a,b) do\
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

    return r;
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
