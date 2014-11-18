#include <stdio.h>
#include <stdlib.h>
#include "web.h"

#define LIBRARY_VERSION 0.1

/**
 * Report library version.
 *
 * @return double
 */
var version()
{
    var v = {0};

    v.type = VAR_DOUBLE;
    v.data_size = sizeof(double);
    v.data = malloc(sizeof(double));
    *(double *)v.data = LIBRARY_VERSION;

    return v;
}

/**
 * Render an HTML template using optional parameters.
 * No control statement support in the first version.
 *
 * @param string filename, map variables
 * @return string
 */
var render(BYTE argc, var *stack)
{
/*    // TODO: use some predefined path
    FILE *html = fopen(template->data, "rt");
    if (!html)
    {
        // TODO: report an error
    }

    // 1. Replace variables

    fclose(html);*/

    return stack[0];
}

/**
 * Generates a hash for a password.
 *
 * @param string password, string salt
 * @return string
 */
var secret(BYTE argc, var *stack)
{
    var v = {0};

    return v;
}

/**
 * Extracts a parameter value by its name from HTTP posted body.
 *
 * @param string password, string salt
 * @return mixed
 */
var fromPost(BYTE argc, var *stack)
{
    var v = {0};

    return v;
}
