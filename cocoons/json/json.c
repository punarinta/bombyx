#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "json.h"

#define LIBRARY_VERSION 0.1

/**
 * Report library version.
 *
 * @return double
 */
var version_()
{
    var v = {0};

    v.type = VAR_DOUBLE;
    v.data_size = sizeof(double);
    v.data = malloc(sizeof(double));
    *(double *)v.data = LIBRARY_VERSION;

    return v;
}

/**
 * Converts a Bombyx variable to a JSON string.
 *
 * @param mixed input
 * @return string
 */
var to_(bombyx_env_t *env, BYTE argc, var *stack)
{
    var v = {0};

    return v;
}

/**
 * Converts a JSON string to a Bombyx variable.
 *
 * @param string input
 * @return mixed
 */
var from_(bombyx_env_t *env, BYTE argc, var *stack)
{
    if (argc != 1 || stack[0].type != VAR_STRING)
    {
        return cocoon_error(env, "Parameters should be of type STRING.");
    }

    json_error_t error;
    json_t *j = json_loads(stack[0].data, 0, &error);  // JSON_DECODE_ANY

    if (!j)
    {
        larva_error(env, "Cannot decode JSON variable '%s'.", stack[0].data);
    }

    var v = {0};

    if (json_is_object(j))
    {
        v.type = VAR_MAP;
        v.data = json_to_map(env, j);
        v.data_size = sizeof(map_table_t);
    }
    else if (json_is_array(j))
    {
        v.type = VAR_ARRAY;
        v.data = json_to_array(env, j);
        v.data_size = sizeof(array_t);
    }
    else if (json_is_string(j))
    {
        v.type = VAR_STRING;
        v.data = strdup(json_string_value(j));
        v.data_size = json_string_length(j) + 1;
    }
    else if (json_is_number(j))
    {
        v.type = VAR_DOUBLE;
        v.data = challoc(env->pool_of_doubles);
        v.data_size = sizeof(double);
        *(double *)v.data = json_number_value(j);
    }
    else if (json_is_true(j))
    {
        v.type = VAR_DOUBLE;
        v.data = challoc(env->pool_of_doubles);
        v.data_size = sizeof(double);
        *(double *)v.data = 1;
    }
    else if (json_is_false(j))
    {
        v.type = VAR_DOUBLE;
        v.data = challoc(env->pool_of_doubles);
        v.data_size = sizeof(double);
        *(double *)v.data = 0;
    }
    else if (json_is_null(j))
    {
        v.type = VAR_UNSET;
    }
    else
    {
        // unsupported data type?
        larva_error(env, "JSON parsing error");
    }

    json_decref(j);

    return v;
}
