#include "array.h"

array_t *array_create(size_t size)
{
    array_t *array = malloc(sizeof(array_t));

    array->size = size;
    array->vars = malloc(sizeof(var*) * size);

    return array;
}

array_t *array_clone(array_t *array)
{
    array_t *new_array = malloc(sizeof(array_t));

    new_array->size = array->size;
    new_array->vars = malloc(sizeof(var*) * array->size);

    if (array->vars)
    {
        for (size_t i = 0; i < array->size; ++i)
        {
            if (array->vars[i]) op_copy(new_array->vars[i], array->vars[i]);
            else new_array->vars[i] = NULL;
        }
    }

    return new_array;
}

void array_delete(array_t *array)
{
    if (!array) return;

    if (array->vars)
    {
        // kill all vars
        for (size_t i = 0; i < array->size; ++i)
        {
            if (array->vars[i]) var_unset(array->vars[i]);
        }

        free(array->vars);
    }

    free(array);
}
