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

int var_set_by_index(unsigned int i, var o)
{
    if (!i || i >= vars_count || vars[i].type == VAR_UNSET) return 0;

    // deallocate memory of the old variable
    if (has_data(vars[i])) free(vars[i].data);

    vars[i].type = o.type;
    vars[i].data_size = o.data_size;
    if (has_data(o) && o.data_size)
    {
        vars[i].data = malloc(o.data_size);
        memcpy(vars[i].data, o.data, o.data_size);
        free(o.data);
    }
    else vars[i].data = NULL;

    return 1;
}

var var_as_double(double a)
{
    var v;
    v.name = NULL;
    v.type = VAR_DOUBLE;
    v.data = malloc(sizeof(double) + 1);
    v.data[0] = VAR_SIGNATURE;
    memcpy(v.data + 1, &a, sizeof(double));
    v.data_size = sizeof(double) + 1;

    return v;
}

var var_set_double(var v, double a)
{
    if (has_data(v)) free(v.data);
    v.type = VAR_DOUBLE;
    v.data = malloc(sizeof(double) + 1);
    v.data[0] = VAR_SIGNATURE;
    memcpy(v.data + 1, &a, sizeof(double));
    v.data_size = sizeof(double) + 1;

    return v;
}

var var_as_string(char *a)
{
    var v;
    v.name = NULL;
    v.type = VAR_STRING;

    unsigned int len = strlen(a);
    v.data = malloc(sizeof(char) * (len + 2));
    memcpy(v.data + 1, a, sizeof(char) * len);
    v.data[0] = VAR_SIGNATURE;

    len++;
    v.data[len++] = '\0';
    v.data_size = len;

    return v;
}

var var_set_string(var v, char *a)
{
    if (has_data(v)) free(v.data);
    v.type = VAR_STRING;
    unsigned int len = strlen(a);
    v.data = malloc(sizeof(char) * (len + 2));
    memcpy(v.data + 1, a, sizeof(char) * len);
    v.data[0] = VAR_SIGNATURE;

    len++;
    v.data[len++] = '\0';
    v.data_size = len;

    return v;
}

double var_to_double(var a)
{
    double d;

    if (!a.data || a.data[0] != VAR_SIGNATURE) return 0;

    if (a.type == VAR_DOUBLE) memcpy(&d, a.data + 1, sizeof(double));
    if (a.type == VAR_STRING) d = atof(a.data);

    if (!a.name) free(a.data);

    return d;
}

double var_extract_double(var a)
{
    double d;

    if (!a.data) return 0;

    if (a.type == VAR_DOUBLE) memcpy(&d, a.data + 1, sizeof(double));
    if (a.type == VAR_STRING) d = atof(a.data);

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

void var_free(var a)
{
    if (a.name) free(a.name);
    if (has_data(a)) free(a.data);
}

void var_delete_by_index(unsigned int index)
{
    if (index && index < vars_count)
    {
        if (vars[index].name) { free(vars[index].name); vars[index].name = NULL; }
        if (has_data(vars[index])) { free(vars[index].data); vars[index].data = NULL; }
        vars[index].type = VAR_UNSET;
    }
}

inline BYTE has_data(var v)
{
    return (v.data && v.data[0] == VAR_SIGNATURE);
}

void var_echo(var a)
{
    if (a.data && a.data[0] != VAR_SIGNATURE) fprintf(stdout, "corrupted!");
    else switch (a.type)
    {
        case VAR_STRING:
        if (a.data && a.data_size) fprintf(stdout, "%s", a.data + 1);
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
        fprintf(stdout, "%.6g", var_extract_double(a));
        break;

        default:
        break;
    }
}