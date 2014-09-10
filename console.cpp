#include <stdio.h>
#include <stdlib.h>

void larva_digest(char *);

int main (void)
{
    char *source = NULL;

    FILE *fp = fopen("tests/test-01.leaf", "rt");

    if (fp != NULL)
    {
        /* Go to the end of the file. */
        if (fseek(fp, 0L, SEEK_END) == 0)
        {
            long bufsize = ftell(fp);
            if (bufsize == -1) { /* Error */ }

            source = (char *) malloc(sizeof(char) * (bufsize + 1));

            if (fseek(fp, 0L, SEEK_SET) != 0) { /* Error */ }

            size_t newLen = fread(source, sizeof(char), bufsize, fp);
            if (newLen == 0)
            {
                fputs("Error reading file. Is it empty?\n", stderr);
            }
            else
            {
                source[++newLen] = '\0';
            }
        }

        larva_digest(source);

        fclose(fp);
    }

    free(source);

    return 0;
}