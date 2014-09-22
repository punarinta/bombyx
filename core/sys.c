#include <sys/time.h>
#include "../common.h"

/**
 *  Trim spaces here and there
 */
char *trim(char *str)
{
    size_t len = 0;
    char *frontp = str - 1;
    char *endp = NULL;

    if (str == NULL) return NULL;
    if (str[0] == '\0') return str;

    len = strlen(str);
    endp = str + len;

    /* Move the front and back pointers to address
     * the first non-whitespace characters from
     * each end.
     */
    while (isspace(*(++frontp)));
    while (isspace(*(--endp)) && endp != frontp);

    if (str + len - 1 != endp ) *(endp + 1) = '\0';
    else if (frontp != str &&  endp == frontp) *str = '\0';

    /* Shift the string so that it starts at str so
     * that if it's dynamically allocated, we can
     * still free it on the returned pointer.  Note
     * the reuse of endp to mean the front of the
     * string buffer now.
     */
    endp = str;

    if (frontp != str)
    {
        while (*frontp) *endp++ = *frontp++;
        *endp = '\0';
    }

    return str;
}

char *strdup(const char *s)
{
    size_t len = strlen(s);
    char *p = malloc(++len);
    return p ? memcpy(p, s, len) : NULL;
}

double get_microtime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000L + tv.tv_usec;
}

void *bytecopy(void *const dest, void const *const src, size_t bytes)
{
    while (bytes-- > (size_t)0) ((unsigned char *)dest)[bytes] = ((unsigned char const *)src)[bytes];
    return dest;
}