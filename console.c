#include <stdio.h>
#include <stdlib.h>
#include "common.h"

void larva_init(char *, unsigned int);
int larva_digest_start();

int main(int argc, char *argv[])
{
    char *source = NULL;
    size_t newLen = 0;
    verbose = 0;

    if (argc < 2)
    {
        fputs("Usage: bombyx [FILENAME] [OPTIONS]\n\n", stderr);
        return 1;
    }

    for (int i = 2; i < argc; i++)
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

        fclose(fp);

        larva_init(source, newLen);
        larva_digest_start();
    }

    free(source);

#ifdef __linux__
    muntrace();
#endif

    return 0;
}