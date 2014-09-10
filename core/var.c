#include "var.h"
#include "../larva.h"

size_t var_add(char *name, unsigned short type, void *value)
{
    // find first empty
    unsigned long l = vars_count / 2 + 1, i;

    for (i = 0; i < vars_count; i++)
    {
        if (vars[i].name == NULL)
        {
            break;
        }
        if (vars[vars_count - i - 1].type != 0)
        {
            i = vars_count - i - 1;
            break;
        }
    }

    unsigned long len = strlen(value);

    // TODO: check length

    vars[i].name = malloc(sizeof(char) * len);
    strcpy(vars[i].name, value);

    return 0;
}

void var_delete(char *name)
{

}

/*void var_delete_by_index(unsigned long index)
{
    if (index >= vars_count) return;

    vars[index]->type = 0;
    strcpy(vars[index]->name, '\0');

    if (vars[index]->data)
    {
        free(vars[index]->data);
    }
}*/
