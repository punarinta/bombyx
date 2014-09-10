#include <stdio.h>
#include <stdlib.h>

void larva_init();
int larva_digest(char *, size_t);

int main (void)
{
    char *source = NULL;
    size_t newLen = 0;

    FILE *fp = fopen("tests/1.leaf", "rt");

    if (fp != NULL)
    {
        /* Go to the end of the file. */
        if (fseek(fp, 0L, SEEK_END) == 0)
        {
            long bufsize = ftell(fp);
            if (bufsize == -1) { /* Error */ }

            source = malloc(sizeof(char) * (bufsize + 1));

            if (fseek(fp, 0L, SEEK_SET) != 0) { /* Error */ }

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

        larva_init();
        larva_digest(source, newLen);

        fclose(fp);
    }

    free(source);

    return 0;
}