#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "larva.h"

int main(int argc, char *argv[])
{
/*
    // TESTING AREA

    #include <sys/time.h>
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double t1 = tv.tv_sec * 1000000L + tv.tv_usec;

    char *a; int n = 1000000, b = n;
    while (b--)
    {
        a = malloc(28);
        free(a);
    }

    gettimeofday(&tv, NULL);
    double t2 = tv.tv_sec * 1000000L + tv.tv_usec;

    printf("\n1 malloc-free consumes %.6g us\n", (t2 - t1)/n);
    return 0;

    // END OF TESTING AREA
*/

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

        fclose(fp);

        setjmp(error_exit);

        if (gl_error)
        {
            larva_stop();
        }
        else
        {
            larva_init(source, newLen);
            var_free(larva_digest());
            larva_stop();
        }
    }

    free(source);

#ifdef __linux__
    muntrace();
#endif

    return gl_error;
}