#include <stdio.h>
#include <stdlib.h>

void larva_init(char *, unsigned int);
int larva_digest();

int main(int argc, char *argv[])
{
    char *source = NULL;
    size_t newLen = 0;

    if (argc < 2)
    {
        fputs("Usage: bombyx [FILENAME]\n\n", stderr);
        return 1;
    }

#ifndef __APPLE__
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
        larva_digest();
    }

    free(source);

#ifndef __APPLE__
    muntrace();
#endif

    return 0;
}