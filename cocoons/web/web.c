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
    dir_leaf_temp = dirname(FCGX_GetParam("SCRIPT_FILENAME", env->request.envp));
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
 * Generates a hash for a password.
 *
 * @param string password
 * @param string salt
 *
 * @return string
 */
var secret_(bombyx_env_t *env, BYTE argc, var *stack)
{
    if (argc < 1 || stack[0].type != VAR_STRING)
    {
        return cocoon_error(env, "Password must be of type STRING.");
    }
    if (argc < 2 || stack[1].type != VAR_STRING)
    {
        return cocoon_error(env, "Salt must be of type STRING.");
    }

    var v = {0};
    v.type = VAR_STRING;
    v.data_size = BCRYPT_HASHSIZE;
    v.data = malloc(BCRYPT_HASHSIZE);
    bcrypt_hashpw(stack[0].data, stack[1].data, v.data);

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
    var v = {0};
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
