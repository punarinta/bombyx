#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include <fcntl.h>
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
 * @param string filename
 * @param map variables
 * @return string
 */
var render(FCGX_Request *request, BYTE argc, var *stack)
{
    var null_var = {0};

    char *dir_leaf_temp, dir_home[1024], dir_leaf[1024];

    getcwd(dir_home, sizeof(dir_home));
    dir_leaf_temp = dirname(FCGX_GetParam("SCRIPT_FILENAME", request->envp));
    strcpy(dir_leaf, dir_leaf_temp);
    chdir(dir_leaf);

    // variables - stack[1];
    // options   - stack[2];

    char *html = get_file_contents(stack[0].data);

    if (stack[1].type != VAR_MAP)
    {
        return null_var;
    }

    if (html)
    {
        if (request) FCGX_PutS(html, request->out);
        else puts(html);
        free(html);
    }

    chdir(dir_home);

    return null_var;
}

/**
 * Generates a hash for a password.
 *
 * @param string password
 * @param string salt
 * @return string
 */
var secret(FCGX_Request *request, BYTE argc, var *stack)
{
    var v = {0};

    return v;
}

/**
 * Extracts a parameter value by its name from HTTP posted body.
 *
 * @param string key
 * @return mixed
 */
var fromPost(FCGX_Request *request, BYTE argc, var *stack)
{
    var v = {0};

    return v;
}
