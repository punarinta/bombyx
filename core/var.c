#include "var.h"

unsigned int var_init(char *name, unsigned short type, void *value)
{
    unsigned int i;

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

int var_set_by_index(unsigned int i, var o, int OBSOLETE)
{
    if (!i || i >= vars_count || vars[i].type == VAR_UNSET) return 0;

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
    v.data = malloc(sizeof(double));
    memcpy(v.data, &a, sizeof(double));
    v.data_size = sizeof(double);

    return v;
}

var var_set_double(var v, double a)
{
    if (v.data) free(v.data);
    v.type = VAR_DOUBLE;
    v.data = malloc(sizeof(double));
    memcpy(v.data, &a, sizeof(double));
    v.data_size = sizeof(double);

    return v;
}

var var_as_dword(double a)
{
    var v;
    v.type = VAR_DWORD;
    v.data = malloc(4);
    memcpy(v.data, &a, 4);
    v.data_size = 4;

    return v;
}

var var_as_string(char *a)
{
    var v;
    v.type = VAR_STRING;
    unsigned int len = strlen(a) + 1;
    v.data = calloc(len, sizeof(char));
    memcpy(v.data, a, sizeof(char) * len);
    v.data_size = sizeof(char) * len;

    return v;
}

var var_set_string(var v, char *a)
{
    if (v.data) free(v.data);
    v.type = VAR_STRING;
    unsigned int len = strlen(a) + 1;
    v.data = calloc(len, sizeof(char));
    memcpy(v.data, a, sizeof(char) * len);
    v.data_size = sizeof(char) * len;

    return v;
}

double var_to_double(var a)
{
    double d;

    if (a.type == VAR_DOUBLE) memcpy(&d, a.data, sizeof(double));
    if (a.type == VAR_STRING) d = atof(a.data);

  //  if (!a.name) free(a.data);

    return d;
}

// not sure if ever used
unsigned int var_to_dword(var a)
{
    DWORD d;
    memcpy(&d, a.data, 4);
    return d;
}

unsigned int var_get_index(char *name)
{
    for (unsigned int i = 1; i < vars_count; i++)
    {
        if (vars[i].type != VAR_UNSET && !strcmp(vars[i].name, name))
        {
            return i;
        }
    }

    return 0;
}

void var_delete_by_index(unsigned int index)
{
    if (index && index < vars_count)
    {
        if (vars[index].name) free(vars[index].name);
        vars[index].type = VAR_UNSET;
        if (vars[index].data) free(vars[index].data);
    }
}

void var_echo(var a)
{
    switch (a.type)
    {
        case VAR_STRING:
        if (a.data && a.data_size) fprintf(stdout, "%s", a.data);
        else fprintf(stdout, "NULL");
        break;

        case VAR_BYTE:
        fprintf(stdout, "%ud", a.data[0]);
        break;

        case VAR_WORD:
        fprintf(stdout, "%ud", (unsigned) (a.data[0] + 256 * a.data[1]));
        break;

        case VAR_DWORD:
        fprintf(stdout, "%ud", (unsigned) (a.data[0] + 256 * a.data[1] + 65536 * a.data[2] + 16777216 * a.data[3]));
        break;

        case VAR_DOUBLE:
        fprintf(stdout, "%.6g", var_to_double(a));
        break;

        default:
        break;
    }
}