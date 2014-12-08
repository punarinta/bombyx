#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "web.h"
#include "bcrypt.h"

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
 * Render an HTML template using optional parameters.
 * No control statement support in the first version.
 *
 * @param string filename
 * @param map variables
 *
 * @return string
 */
var render_(bombyx_env_t *env, BYTE argc, var *stack)
{
    // TODO: CLI support
    var null_var = {0};

    char *dir_leaf_temp, dir_home[1024], dir_leaf[1024];

    if (!env->request.out)
    {
        return cocoon_error(env, "Rendering disabled for CLI.");
    }

    getcwd(dir_home, sizeof(dir_home));
    dir_leaf_temp = dirname(FCGX_GetParam("LEAF_FILENAME", env->request.envp));
    strcpy(dir_leaf, dir_leaf_temp);
    chdir(dir_leaf);

#ifdef __APPLE__
    //free(dir_leaf_temp);
#endif

    // variables - stack[1];
    // options   - stack[2];

    char *html = get_file_contents(stack[0].data);

    if (argc == 2 && stack[1].type != VAR_MAP)
    {
        chdir(dir_home);
        return cocoon_error(env, "Parameters should be of type MAP.");
    }

    if (html && html[0] && html[1])    // at least 2 byte are present
    {
        replace:;
        int i = 1;
        char *token, *search, *frame = NULL;
        size_t frame_length = 0;

        while (html[i])
        {
            if (frame) ++frame_length;
            if (html[i - 1] == '{' && html[i - 2] == '{') frame = html + i;
            if (html[i] == '}' && html[i - 1] == '}')
            {
                // form a search string
                search = malloc(frame_length + 4);
                memcpy(search, frame - 2, frame_length + 3);
                search[frame_length + 3] = '\0';

                // find a token name
                --frame_length;
                token = malloc(frame_length);
                memcpy(token, frame, --frame_length);
                token[frame_length] = '\0';
                trim(token);
                frame = NULL;

                map_t *m = map_lookup((map_table_t *)stack[1].data, token);
                if (m && m->v.type)
                {
                    html = str_replace(html, search, m->v.data);
                    free(token);
                    free(search);
                    goto replace;
                }
            }
            ++i;
        }

        FCGX_PutS(html, env->request.out);
        free(html);
    }

    chdir(dir_home);

    return null_var;
}

/**
 * Reads / writes a cookie.
 *
 * @param string name
 * @param [string value]
 * @param [map options]
 *
 * @return string
 */
var cookie_(bombyx_env_t *env, BYTE argc, var *stack)
{
    if (argc < 1 || stack[0].type != VAR_STRING)
    {
        return cocoon_error(env, "No key given or type is not [STRING].");
    }

    var v = {0};

    if (argc > 1)
    {
        // TODO: support more convenient way of setting cookies
        FCGX_FPrintF(env->request.out, "Set-Cookie: %s=%s;\r\n", stack[0].data, stack[1].data);
        return v;
    }

    // explode COOKIE string
    char *key, *val, *tok = NULL, *query = strdup(FCGX_GetParam("HTTP_COOKIE", env->request.envp));

    tok = strtok(query, ";");

    while (tok)
    {
        char *pair = strdup(tok);
        char *saveptr;

        key = strtok_r(pair, "=", &saveptr);
        val = strtok_r(NULL, "=", &saveptr);

        if (!memcmp(stack[0].data, key, stack[0].data_size))
        {
            // key found, return
            v.type = VAR_STRING;
            v.data = strdup(val);
            v.data_size = strlen(v.data) + 1;
            free(pair);
            return v;
        }

        free(pair);
        tok = strtok(NULL, ";");

        if (!tok) break;
    }

    return v;
}

/**
 * Generates a hash for a password.
 *
 * @param string password
 * @param mixed salt|strength
 *
 * @return string
 */
var secret_(bombyx_env_t *env, BYTE argc, var *stack)
{
    if (argc < 2 || stack[0].type != VAR_STRING)
    {
        return cocoon_error(env, "Password [STRING] of a wrong type or too few arguments.");
    }

    var v = {0};
    v.type = VAR_STRING;
    v.data_size = BCRYPT_HASHSIZE;
    v.data = malloc(BCRYPT_HASHSIZE);

    if (stack[1].type == VAR_DOUBLE)
    {
        // generate salt
        char salt[BCRYPT_HASHSIZE];
        bcrypt_gensalt((int)*(double *)stack[1].data, salt);
        bcrypt_hashpw(stack[0].data, salt, v.data);
    }
    else if (stack[1].type == VAR_STRING)
    {
        // use salt
        bcrypt_hashpw(stack[0].data, stack[1].data, v.data);
    }
    else
    {
        return cocoon_error(env, "Second argument should be either a salt [STRING] or a strength [NUMBER].");
    }

    return v;
}

/**
 * Extracts a parameter value by its name from HTTP GET query.
 *
 * @param string key
 *
 * @return mixed
 */
var fromGet_(bombyx_env_t *env, BYTE argc, var *stack)
{
    if (argc < 1 || stack[0].type != VAR_STRING)
    {
        return cocoon_error(env, "No key [STRING] given.");
    }

    var v = {0};

    // explode GET
    char *key, *val, *tok = NULL, *query = strdup(FCGX_GetParam("QUERY_STRING", env->request.envp));

    tok = strtok(query, "&");

    while (tok)
    {
        char *pair = strdup(tok);
        char *saveptr;

        key = strtok_r(pair, "=", &saveptr);
        val = strtok_r(NULL, "=", &saveptr);

        if (!memcmp(stack[0].data, key, stack[0].data_size))
        {
            // key found, return
            v.type = VAR_STRING;
            v.data = strdup(val);
            v.data_size = strlen(v.data) + 1;
            free(pair);
            return v;
        }

        free(pair);
        tok = strtok(NULL, "&");

        if (!tok) break;
    }

    return v;
}

/**
 * Extracts a parameter value by its name from HTTP POST body.
 *
 * @param string key
 *
 * @return mixed
 */
var fromPost_(bombyx_env_t *env, BYTE argc, var *stack)
{
    var v = {0};

    if (!env->http_length)
    {
        // just return NULL
        return v;
    }

    v.type = VAR_STRING;
    v.data_size = env->http_length + 1;
    v.data = strdup(env->http_content);

    return v;
}

/**
 * Extracts a parameter value by its name from HTTP POST body posted as JSON.
 *
 * @param string key
 *
 * @return mixed
 */
var fromJson_(bombyx_env_t *env, BYTE argc, var *stack)
{
    var v = {0};

    if (!env->http_length)
    {
        // just return NULL
        return v;
    }

    json_error_t error;
    json_t *jo = json_loads(env->http_content, 0, &error), *j;

    if (!jo)
    {
        return cocoon_error(env, "Cannot decode JSON variable '%s'.", env->http_content);
    }

    if (argc == 1)
    {
        // get an element

        if (stack[0].type == VAR_STRING)
        {
            // TODO: support APath
            j = json_object_get(jo, stack[0].data);
        }
        else if (stack[0].type == VAR_DOUBLE)
        {
            j = json_array_get(jo, (int)*(double*)stack[0].data);
        }
        else
        {
            return cocoon_error(env, "Element access if not allowed for this type");
        }
    }
    else
    {
        j = jo;
    }

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
        return cocoon_error(env, "JSON parsing error. Unsupported type.");
    }

    json_decref(j);

    return v;
}

/**
 * Performs a GET request.
 *
 * @param string url
 * @param map params
 *
 * @return mixed
 */
var get_(bombyx_env_t *env, BYTE argc, var *stack)
{
    var v = {0};
    return v;
}

/**
 * Performs a POST request.
 *
 * @param string url
 * @param map params
 *
 * @return mixed
 */
var post_(bombyx_env_t *env, BYTE argc, var *stack)
{
    var v = {0};
    return v;
}
