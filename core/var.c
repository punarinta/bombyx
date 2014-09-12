#include "var.h"

unsigned long var_init(char *name, unsigned short type, void *value)
{
    unsigned long i;

    // no necessity to do anything
    if (type == VAR_UNSET) return 0;

    // find first empty
    for (i = 1; i < vars_count; i++) if (vars[i].type == VAR_UNSET) goto OK;

    larva_stop(ERR_NO_MEMORY);

    // TODO: support proper growing

  /*  if (i == vars_count)
    {
        // try to allocate some space
        larva_grow(0);
    }*/

    OK:;

    // save variable name
    size_t len = strlen(name) + 1;
    
    vars[i].name = calloc(len, sizeof(char));

    if (!vars[i].name)
    {
        larva_stop(ERR_NO_MEMORY);
    }

    strcpy(vars[i].name, name);
    vars[i].type = VAR_STRING;
    vars[i].data = NULL;

    return i;
}

int var_set_by_index(unsigned long i, var o, int OBSOLETE)
{
    if (!i || i >= vars_count || vars[i].type == VAR_UNSET) return 0;

    unsigned long y = o.data[0] + o.data[1] * 256;
 //   fprintf(stdout, "setting var #%lu to %lu\n", i, y);

    // deallocate memory of the old variable
    if (vars[i].data) free(vars[i].data);

    vars[i].type = o.type;
    vars[i].data = o.data;
    vars[i].data_size = o.data_size;

    return 1;
}

var var_as_double(double a)
{
    var v;
    v.type = VAR_DOUBLE;
    v.data = calloc(1, sizeof(double));
    memcpy(v.data, &a, sizeof(double));
    v.data_size = sizeof(double);

    return v;
}

var var_as_dword(double a)
{
    var v;
    v.type = VAR_DWORD;
    v.data = calloc(1, 4);
    memcpy(v.data, &a, 4);
    v.data_size = 4;

    return v;
}

var var_as_string(char *a)
{
    var v;
    v.type = VAR_STRING;
    unsigned len = strlen(a) + 1;
    v.data = calloc(len, sizeof(char));
    memcpy(v.data, a, sizeof(char) * len);
    v.data_size = sizeof(char) * len;

    return v;
}

double var_to_double(var a)
{
    double d;
    memcpy(&d, a.data, sizeof(double));
    return d;
}

unsigned int var_to_dword(var a)
{
    unsigned int d;
    memcpy(&d, a.data, 4);
    return d;
}

var var_add(var a, var b)
{
    double xa, xb;
    memcpy(&xa, a.data, sizeof(double));
    memcpy(&xb, b.data, sizeof(double));

    xa += xb;
    free(b.data);

    memcpy(a.data, &xa, sizeof(double));

    return a;
}

var var_subtract(var a, var b)
{
    double xa, xb;
    memcpy(&xa, a.data, sizeof(double));
    memcpy(&xb, b.data, sizeof(double));

    xa -= xb;
    free(b.data);

    memcpy(a.data, &xa, sizeof(double));

    return a;
}

var var_multiply(var a, var b)
{
    double xa, xb;
    memcpy(&xa, a.data, sizeof(double));
    memcpy(&xb, b.data, sizeof(double));

    xa *= xb;
    free(b.data);

    memcpy(a.data, &xa, sizeof(double));

    return a;
}

var var_divide(var a, var b)
{
    double xa, xb;
    memcpy(&xa, a.data, sizeof(double));
    memcpy(&xb, b.data, sizeof(double));

    xa /= xb;
    free(b.data);

    memcpy(a.data, &xa, sizeof(double));

    return a;
}

var var_invert(var a)
{
    double x = -(*a.data);
    memcpy(a.data, &x, sizeof(double));

    return a;
}

int var_is_more(var a, var b)
{
    return (*a.data) > (*b.data);
}

int var_is_less(var a, var b)
{
    return (*a.data) < (*b.data);
}

int var_is_more_equal(var a, var b)
{
    return (*a.data) >= (*b.data);
}

int var_is_less_equal(var a, var b)
{
    return (*a.data) <= (*b.data);
}

var var_array_element(var a, unsigned int i)
{
    // TODO
    a = var_as_double(42);
    return a;
}

unsigned long var_get_index(char *name)
{
    for (unsigned long i = 1; i < vars_count; i++)
    {
        if (vars[i].type != VAR_UNSET && !strcmp(vars[i].name, name))
        {
            return i;
        }
    /*    if (vars[vars_count - i].type != VAR_UNSET && !strcmp(vars[vars_count - i - 1].name, name))
        {
            return vars_count - i;
        }*/
    }

  //  fputs(" var not found ", stdout);
    return 0;
}

void var_delete(char *name)
{
    var_delete_by_index(var_get_index(name));
}

void var_delete_by_index(unsigned long index)
{
    if (index && index < vars_count)
    {
        if (vars[index].name) free(vars[index].name);
        vars[index].type = VAR_UNSET;
        if (vars[index].data) free(vars[index].data);
    }
}