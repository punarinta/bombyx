#include "common.h"
#include "array.h"
#include "var_2.h"

#define MIN_ARRAY_SIZE 100

array_t *array_create(size_t size)
{
    if (!size)
        size = MIN_ARRAY_SIZE;

    array_t *array = malloc(sizeof(array_t));

    array->size = 0;
    array->max_size = size;
    array->vars = malloc(sizeof(var*) * size);

    return array;
}

/*
    Warning: array_push() uses direct variable copying, not an op_copy()
*/
void array_push(array_t *array, var v)
{
    if (array->size >= array->max_size)
    {
        if (array->max_size < MIN_ARRAY_SIZE)
            array->max_size = MIN_ARRAY_SIZE * 2;
        else
            array->max_size *= 2;

        array->vars = realloc(array->vars, sizeof(var*) * array->max_size);
        if (!array->vars)
        {
            // do something!
        }
    }

    // note that 'v' is not a pointer, so it can be safely stored in the array
    var *pv = malloc(sizeof(var));
    *pv = v;
    array->vars[array->size++] = pv;
}

/*
    Warning: array_set_elem() uses direct variable copying, not an op_copy()
*/
void array_set_elem(bombyx_env_t *env, array_t *array, unsigned int index, var v)
{
    if (index >= array->size)
    {
        if (index >= array->max_size)
        {
            array->max_size = MAX(index + 1, array->max_size * 2);
            array->vars = realloc(array->vars, sizeof(var*) * array->max_size);
        }

        // element does not exist
        var *pv = malloc(sizeof(var));
        *pv = v;
        array->vars[index] = pv;

        // fill all the orphan links with nulls
        for (int i = array->size; i < index; i++)
        {
            array->vars[i] = NULL;
        }

        array->size = index + 1;
    }
    else
    {
        // element already exists
        *array->vars[index] = v;
    }
}

array_t *array_clone(bombyx_env_t *env, array_t *array)
{
    array_t *new_array = malloc(sizeof(array_t));

    new_array->size = array->size;

    // TODO: choose between assigning to size or max_size
    new_array->max_size = MAX(array->size, MIN_ARRAY_SIZE);
    new_array->vars = malloc(sizeof(var*) * array->max_size);

    if (array->vars)
    {
        for (size_t i = 0; i < array->size; ++i)
        {
            if (array->vars[i])
            {
                new_array->vars[i] = calloc(1, sizeof(var));
                op_copy(env, new_array->vars[i], array->vars[i]);
            }
            else new_array->vars[i] = NULL;
        }
    }

    return new_array;
}

void array_delete(bombyx_env_t *env, array_t *array)
{
    if (!array) return;

    if (array->vars)
    {
        // kill all vars
        for (size_t i = 0; i < array->size; ++i)
        {
            // no necessity to nullify the var, as it will be deleted soon
            if (array->vars[i])
            {
                var_unset(env, array->vars[i]);
                free(array->vars[i]);
            }
        }

        free(array->vars);
    }

    free(array);
}
