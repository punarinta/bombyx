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
        return cocoon_error(request, "Function requires 3 arguments, type STRING.");
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

    if (argc != 1 || stack[0].type != VAR_STRING)
    {
        return cocoon_error(request, "Parameters should be of type STRING.");
    }

    v.data = malloc(stack[0].data_size);
    memcpy(v.data, stack[0].data, stack[0].data_size);
    trim(v.data);
    v.data_size = strlen(v.data) + 1;
    v.type = VAR_STRING;

    return v;
}

var length_(FCGX_Request *request, BYTE argc, var *stack)
{
    var v = {0};

    if (argc != 1 || stack[0].type != VAR_STRING)
    {
        return cocoon_error(request, "Parameters should be of type STRING.");
    }

    v.type = VAR_DOUBLE;
    v.data_size = sizeof(double);
    v.data = malloc(sizeof(double));
    *(double *)v.data = stack[0].data_size - 1;

    return v;
}

var split_(FCGX_Request *request, BYTE argc, var *stack)
{
    var v = {0};
    char *tok = NULL;

    if (argc != 2 || stack[0].type != VAR_STRING)
    {
        return cocoon_error(request, "Function requires 2 arguments, type STRING.");
    }

    v.type = VAR_ARRAY;
    v.data_size = sizeof(array_t);
    v.data = array_create(0);

    tok = strtok(stack[0].data, stack[1].data);

    while (tok)
    {
        var vt = {0};
        vt.type = VAR_STRING;
        vt.data_size = strlen(tok) + 1;
        vt.data = malloc(vt.data_size);
        memcpy(vt.data, tok, vt.data_size);

        array_push(v.data, vt);

        tok = strtok(NULL, stack[1].data);

        if (!tok) break;
    }

    return v;
}