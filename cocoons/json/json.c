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
    var v = {0};

    return v;
}
