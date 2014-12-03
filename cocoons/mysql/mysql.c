#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mysql.h"

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
 * Connects to MySQL.
 *
 * @param string host
 * @param string user
 * @param string password
 * @param string database
 * @param double port
 *
 * @return string
 */
var connect_(bombyx_env_t *env, BYTE argc, var *stack)
{
    if (argc < 4)
    {
        return cocoon_error(env, "Parameters should be of type STRING.");
    }

    var v = {0};
    v.data = mysql_init(NULL);
    if (!mysql_real_connect(v.data, stack[0].data, stack[1].data, stack[2].data, stack[3].data, (unsigned int)(*(double*)stack[4].data), NULL, 0))
    {
    	return cocoon_error(env, "DB error: %s\n", mysql_error(v.data));
    }

    v.type = VAR_CUSTOM;
    v.data_size = sizeof(MYSQL);

    return v;
}