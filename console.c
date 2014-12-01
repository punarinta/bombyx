#include <stdio.h>
#include <stdlib.h>

// TODO: check that these two exist
#include <libgen.h>
#include <unistd.h>

#include "core/common.h"
#include "core/larva.h"

int main(int argc, char *argv[])
{
    verbose = 0;
    char *source;
    size_t newLen = 0;

    if (argc < 2)
    {
        fputs("Usage: bombyx [FILENAME] [OPTIONS]\n\n", stderr);
        return 1;
    }

    for (int i = 2; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-v"))
        {
            verbose = 1;
        }
    }

    // TODO: save argc/argv into vars

#ifdef BOMBYX_MCHECK
    mtrace();
#endif

    bombyx_env_t *env = calloc(1, sizeof(bombyx_env_t));

    env->gl_error = 0;

    FILE *fp = fopen(argv[1], "rt");

    if (fp != NULL)
    {
        /* Go to the end of the file. */
        if (fseek(fp, 0L, SEEK_END) == 0)
        {
            long bufsize = ftell(fp);
            if (bufsize == -1)
            {
                fputs("Where's the file end?\n", stderr);
                return -1;
            }

            source = malloc(sizeof(char) * (bufsize + 2));

            if (fseek(fp, 0L, SEEK_SET) != 0)
            {
                fputs("Looks like the file has just disappeared.\n", stderr);
            }

            newLen = fread(source, sizeof(char), bufsize, fp);
            if (newLen == 0)
            {
                fputs("Error reading file. Is it empty?\n", stderr);
            }
            else
            {
                source[newLen++] = '\0';
            }
        }
        else
        {
            fputs("File reading error.\n", stderr);
            return -1;
        }

        fclose(fp);

        getcwd(env->dir_home, sizeof(env->dir_home));
        char *dir_leaf_temp = dirname(argv[1]);
        strcpy(env->dir_leaf, dir_leaf_temp);
        chdir(env->dir_leaf);

#ifdef __APPLE__
        free(dir_leaf_temp);
#endif

        setjmp(env->error_exit);

        if (env->gl_error)
        {
            larva_stop(env);
        }
        else
        {
            larva_init(env, source, newLen);
            larva_digest(env);
            larva_silk(env);
            larva_stop(env);
        }
    }
    else
    {
        fputs("File does not exist.\n", stderr);
        if (env) free(env);
        return -1;
    }

    if (env) free(env);

#ifdef BOMBYX_MCHECK
    muntrace();
#endif

    return env->gl_error;
}