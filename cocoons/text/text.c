#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "text.h"

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
 * Performs string replacement.
 *
 * @version 0.1
 *
 * @param string where
 * @param string old
 * @param string new
 *
 * @return mixed
 */
var replace_(FCGX_Request *request, BYTE argc, var *stack)
{
    // TODO: error processing
    // TODO: support replacement without return
    // TODO: support replacement limit
    // TODO: support replacement maps

    if (argc != 3 || stack[0].type != VAR_STRING || stack[1].type != VAR_STRING || stack[2].type != VAR_STRING)
    {
        return cocoon_error(request, "Function requires 3 arguments, type STRING. See manual.");
    }

    var v = {0};
    v.data = str_replace(stack[0].data, stack[1].data, stack[2].data);
    v.data_size = strlen(v.data);
    v.type = VAR_STRING;

    return v;
}

/**
 * Performs string replacement.
 *
 * @version 0.1
 *
 * @param string src
 *
 * @return mixed
 */
var trim_(FCGX_Request *request, BYTE argc, var *stack)
{
    var v = {0};

    if (argc == 1 && stack[0].type == VAR_STRING)
    {
        v.data = malloc(stack[0].data_size);
        memcpy(v.data, stack[0].data, stack[0].data_size);
        trim(v.data);
        v.data_size = strlen(v.data) + 1;
        v.type = VAR_STRING;
    }

    return v;
}
