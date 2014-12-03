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
        return cocoon_error(env, "Some crap with function arguments, see manual.");
    }

    unsigned int port = 0;
    if (argc > 4)
    {
        port = *(double*)stack[4].data;
    }

    var v = {0};
    v.data = mysql_init(NULL);
    if (!mysql_real_connect(v.data, stack[0].data, stack[1].data, stack[2].data, stack[3].data, port, NULL, 0))
    {
    	return cocoon_error(env, "MySQL connection error: %s", mysql_error(v.data));
    }

    v.type = VAR_CUSTOM;
    v.data_size = sizeof(MYSQL);

    return v;
}

/**
 * Connects to MySQL.
 *
 * @param custom sql-connection
 * @param string query
 *
 * @return string
 */
var query_(bombyx_env_t *env, BYTE argc, var *stack)
{
    if (argc < 2 || stack[1].type != VAR_STRING)
    {
        return cocoon_error(env, "Second parameter should be of type STRING.");
    }

	if (mysql_query(stack[0].data, stack[1].data))
	{
		return cocoon_error(env, "MySQL query error: %s", mysql_error(stack[0].data));
	}

	var v = {0};

    v.type = VAR_CUSTOM;
    v.data = mysql_use_result(stack[0].data);
    v.data_size = sizeof(MYSQL_RES);

    return v;
}

/**
 * Connects to MySQL.
 *
 * @param custom sql-result
 * @param string query
 *
 * @return string
 */
var fetchRow_(bombyx_env_t *env, BYTE argc, var *stack)
{
    if (argc < 1)
    {
        return cocoon_error(env, "No pointer to SQL result given.");
    }

	var v = {0};
	MYSQL_ROW row;
	row = mysql_fetch_row(stack[0].data);
    unsigned int num_rows = mysql_num_fields(stack[0].data);

    v.type = VAR_ARRAY;
    v.data = array_create(num_rows);
    v.data_size = sizeof(array_t);

    for (unsigned int i = 0; i < num_rows; i++)
    {
        var v2 = {0};
        v.type = VAR_STRING;
        v.data_size = strlen(row[i]) + 1;
        v.data = strdup(row[i]);

        array_push(v.data, v2);
    }

    return v;
}
