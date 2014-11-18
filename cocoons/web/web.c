#include <stdio.h>
#include <stdlib.h>
#include "web.h"

#define LIBRARY_VERSION 0.1

/*
    Report library version.
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

/*
    Render an HTML template using optional parameters.
    No control statement support in the first version.
*/
var render(BYTE argc, var *argv)
{
    // TODO: use some predefined path
    FILE *html = fopen(template->data, "rt");
    if (!html)
    {
        // TODO: report an error
    }

    // 1. Replace variables

    fclose(html);

    return v;
}