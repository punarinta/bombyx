#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "core/larva.h"

int main(int argc, char *argv[])
{
    verbose = 0;
    gl_error = 0;
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

#ifdef __linux__
    mtrace();
#endif

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

            source = malloc(sizeof(char) * (bufsize + 1));

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
                source[++newLen] = '\0';
            }
        }
        else
        {
            fputs("File reading error.\n", stderr);
            return -1;
        }

        // fclose(fp);

        setjmp(error_exit);

        if (gl_error)
        {
            larva_stop();
        }
        else
        {
            larva_init(source, newLen);
            larva_digest();

            larva_silk();

            larva_stop();
        }
    }

    free(source);

#ifdef __linux__
    muntrace();
#endif

    return gl_error;
}