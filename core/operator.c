#include "var.h"

var var_assign(var a, var b)
{
    // 'a' will be overwritten in any case
    if (has_data(a)) free(a.data);

    a.data = malloc(b.data_size);
    memcpy(a.data, b.data, b.data_size);

    a.type = b.type;
    a.data_size = b.data_size;

    if (!b.name)
    {
        if (has_data(b)) free(b.data);
    }
    if (gl_save_names && b.name)
    {
        if (a.name) free(a.name);
        a.name = malloc(strlen(b.name) + 1);
        memcpy(a.name, b.name, strlen(b.name));
        a.name[strlen(b.name)] = '\0';
    }

    if (!b.name && !gl_save_names)
    {
        a.name = NULL;
    }

    return a;
}

/*
    Synchronizes 'a' with vars[] if necessary
*/
void var_sync(var a)
{
    if (a.name)
    {
        unsigned int i = var_get_index(a.name);

        vars[i].type = a.type;
        vars[i].data_size = a.data_size;

        if (vars[i].data) free(vars[i].data);
        vars[i].data = malloc(a.data_size);
        memcpy(vars[i].data, a.data, a.data_size);
    }
}

var var_add(var a, var b)
{
    var r;

    if (a.type == VAR_STRING && b.type == VAR_STRING)
    {
        r.data_size = a.data_size + b.data_size - 2;
        r.data = malloc(r.data_size);
        r.data[0] = VAR_SIGNATURE;
        memcpy(r.data + 1, a.data + 1, a.data_size - 2);
        memcpy(r.data + a.data_size - 1, b.data + 1, b.data_size - 2);

        r.data[r.data_size - 1] = '\0';
        r.type = VAR_STRING;
    }
    else if (a.type == VAR_DOUBLE && b.type == VAR_DOUBLE)
    {
        double xa, xb;
        memcpy(&xa, a.data + 1, sizeof(double));
        memcpy(&xb, b.data + 1, sizeof(double));

        xa += xb;

        r.data = malloc(sizeof(double) + 1);
        r.data[0] = VAR_SIGNATURE;
        memcpy(r.data + 1, &xa, sizeof(double));
        r.type = VAR_DOUBLE;
        r.data_size = sizeof(double) + 1;
    }
    else if (a.type == VAR_STRING && b.type == VAR_DOUBLE)
    {
        double xb;
        char *converted = calloc(256, sizeof(char));        // the length is doubtful
        memcpy(&xb, b.data + 1, sizeof(double));
        sprintf(converted, "%.6g", xb);
        unsigned int len = strlen(converted);

        r.data = malloc(a.data_size + len);
        memcpy(r.data + 1, a.data + 1, a.data_size - 2);
        memcpy(r.data + a.data_size - 1, converted, len);

        r.data[0] = VAR_SIGNATURE;
        r.data_size = a.data_size + len;
        r.data[r.data_size - 1] = '\0';
        r.type = VAR_STRING;

        free(converted);
    }
    else if (a.type == VAR_DOUBLE && b.type == VAR_STRING)
    {
        double xa;
        char *converted = calloc(256, sizeof(char));        // the length is doubtful
        memcpy(&xa, a.data + 1, sizeof(double));
        sprintf(converted, "%.6g", xa);
        unsigned int len = strlen(converted);

        r.data = malloc(a.data_size + len);
        memcpy(r.data + 1, converted, len);
        memcpy(r.data + len + 1, converted, a.data_size - 2);

        r.data[0] = VAR_SIGNATURE;
        r.data_size = a.data_size + len;
        r.data[r.data_size - 1] = '\0';
        r.type = VAR_STRING;

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

    r.name = NULL;

    return r;
}

var var_subtract(var a, var b)
{
    var r = a;

    if (a.type == VAR_DOUBLE && b.type == VAR_DOUBLE)
    {
        double xa, xb;
        memcpy(&xa, a.data + 1, sizeof(double));
        memcpy(&xb, b.data + 1, sizeof(double));
        if (!a.name) free(a.data);
        if (!b.name) free(b.data);

        xa = xa - xb;

        r.data = malloc(sizeof(double) + 1);
        memcpy(r.data + 1, &xa, sizeof(double));
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
    memcpy(&xa, a.data + 1, sizeof(double));
    memcpy(&xb, b.data + 1, sizeof(double));
    if (!a.name) free(a.data);
    if (!b.name) free(b.data);

    xa *= xb;

    r.data = malloc(sizeof(double) + 1);
    memcpy(r.data + 1, &xa, sizeof(double));

    return r;
}

var var_divide(var a, var b)
{
    var r = a;
    double xa, xb;
    memcpy(&xa, a.data + 1, sizeof(double));
    memcpy(&xb, b.data + 1, sizeof(double));
    if (!a.name) free(a.data);
    if (!b.name) free(b.data);

    xa /= xb;

    r.data = malloc(sizeof(double) + 1);
    memcpy(a.data + 1, &xa, sizeof(double));

    return r;
}

var var_invert(var a)
{
    var r = a;

    if (a.type == VAR_DOUBLE)
    {
        double xa;
        memcpy(&xa, a.data + 1, sizeof(double));
        if (!a.name) free(a.data);

        xa = -xa;

        r.data = malloc(sizeof(double) + 1);
        memcpy(r.data + 1, &xa, sizeof(double));
    }
    else if (a.type == VAR_STRING)
    {
        // TODO: debug
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
    if (!a.name)
    {
        fprintf(stderr, "Operator '++' requires a variable.");
        larva_error();
    }
    if (a.type == VAR_DOUBLE)
    {
        double xa;
        memcpy(&xa, a.data + 1, sizeof(double));
        xa++;
        memcpy(a.data + 1, &xa, sizeof(double));
    }
    else
    {
        fprintf(stderr, "Operator '++' is not defined for the given operand type.");
        larva_error();
    }

    var_sync(a);

    return a;
}

var var_decrement(var a)
{
    if (!a.name)
    {
        fprintf(stderr, "Operator '--' requires a variable.");
        larva_error();
    }
    if (a.type == VAR_DOUBLE)
    {
        double xa;
        memcpy(&xa, a.data + 1, sizeof(double));
        xa--;
        memcpy(a.data + 1, &xa, sizeof(double));
    }
    else
    {
        fprintf(stderr, "Operator '--' is not defined for given operand type.");
        larva_error();
    }

    var_sync(a);

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
