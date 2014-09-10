#include "var.h"
#include "../larva.h"

size_t var_add(char *name, unsigned short type, void *value)
{
    unsigned long i;

    if (type == VAR_UNSET)
    {
        return 0;
    }

    // find first empty
    for (i = 0; i < vars_count; i++)
    {
        if (vars[i].name == NULL)
        {
            break;
        }
        if (vars[vars_count - i - 1].name == NULL)
        {
            i = vars_count - i - 1;
            break;
        }
    }

    if (type == VAR_STRING)
    {
        size_t len = strlen(value);

        vars[i].name = malloc(sizeof(char) * len);
        strcpy(vars[i].name, value);

        return len;
    }

    return 0;
}

unsigned long var_get_index(char *name)
{
    for (unsigned long i = 0; i < vars_count; i++)
    {
        if (vars[i].name != NULL && !strcmp(vars[i].name, name))
        {
            return i;
        }
        if (vars[vars_count - i - 1].name != NULL && !strcmp(vars[vars_count - i - 1].name, name))
        {
            i = vars_count - i - 1;
            return i;
        }
    }

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
        free(vars[index].name);
        vars[index].type = VAR_UNSET;
    }
}
