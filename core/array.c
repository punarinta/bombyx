#include "array.h"

array_t *array_create(size_t size)
{
    array_t *array = malloc(sizeof(array_t));

    array->size = size;
    array->count = 0;
    array->vars = malloc(sizeof(var*) * size);

    return array;
}

void array_push(array_t *array, var v)
{
    if (array->count >= array->size)
    {
        array->size *= 2;
        array->vars = realloc(array->vars, sizeof(var*) * array->size);
        if (!array->vars)
        {
            // do something!
        }
    }

    // note that 'v' is not a pointer, so it can be safely stored in the array
    var *pv = malloc(sizeof(var));
    *pv = v;
    array->vars[array->count++] = pv;
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
        for (size_t i = 0; i < array->count; ++i)
        {
            if (array->vars[i]) var_unset(array->vars[i]);
        }

        free(array->vars);
    }

    free(array);
}
