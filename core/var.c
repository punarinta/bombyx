#include "var.h"

unsigned long var_add(char *name, unsigned short type, void *value)
{
    unsigned long i;

    if (type == VAR_UNSET)
    {
        return 0;
    }

    // find first empty
    for (i = 1; i < vars_count; i++)
    {
        if (vars[i].type == VAR_UNSET)
        {
            goto OK;
        }
    /*    if (vars[vars_count - i].type == VAR_UNSET)
        {
            i = vars_count - i;
            goto OK;
        }*/
    }

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
    if (len > 32)
    {
        larva_stop(ERR_TOO_LONG);
    }

    vars[i].name = calloc(len, sizeof(char));

    if (!vars[i].name)
    {
        larva_stop(ERR_NO_MEMORY);
    }

    strcpy(vars[i].name, name);
    vars[i].type = VAR_STRING;

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
    vars[i].data_length = o.data_length;

    return 1;
}

unsigned long var_get_index(char *name)
{
    for (unsigned long i = 1; i < vars_count; i++)
    {
        if (vars[i].type != VAR_UNSET && !strcmp(vars[i].name, name))
        {
         /*   fputs("ID of var '", stdout);
            fputs(name, stdout);
            fputs("' = ", stdout);
            echo_int(i);
            fputs("\n", stdout);*/
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
