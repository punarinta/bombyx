#include <sys/time.h>
#include <stddef.h>
#include "common.h"

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

char *get_file_contents(char *filename)
{
    size_t length;
    char *buffer = NULL;
    FILE *fp = fopen(filename, "rb");

    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        buffer = malloc(length);
        if (buffer)
        {
            fread(buffer, 1, length, fp);
        }
        buffer[length] = 0;
        fclose(fp);
    }

    return buffer;
}

char *str_replace(const char *str, const char *old, const char *new_str)
{
	char *ret, *r;
	const char *p, *q;
	size_t oldlen = strlen(old);
	size_t count, retlen, newlen = strlen(new_str);
	int samesize = (oldlen == newlen);

	if (!samesize)
	{
		for (count = 0, p = str; (q = strstr(p, old)) != NULL; p = q + oldlen)
			count++;
		/* This is undefined if p - str > PTRDIFF_MAX */
		retlen = p - str + strlen(p) + count * (newlen - oldlen);
	}
	else
		retlen = strlen(str);

	if ((ret = malloc(retlen + 1)) == NULL)
		return NULL;

	r = ret, p = str;
	while (1)
	{
		/* If the old and new strings are different lengths - in other
		 * words we have already iterated through with strstr above,
		 * and thus we know how many times we need to call it - then we
		 * can avoid the final (potentially lengthy) call to strstr,
		 * which we already know is going to return NULL, by
		 * decrementing and checking count.
		 */
		if (!samesize && !count--)
			break;
		/* Otherwise i.e. when the old and new strings are the same
		 * length, and we don't know how many times to call strstr,
		 * we must check for a NULL return here (we check it in any
		 * event, to avoid further conditions, and because there's
		 * no harm done with the check even when the old and new
		 * strings are different lengths).
		 */
		if ((q = strstr(p, old)) == NULL)
			break;
		/* This is undefined if q - p > PTRDIFF_MAX */
		ptrdiff_t l = q - p;
		memcpy(r, p, l);
		r += l;
		memcpy(r, new_str, newlen);
		r += newlen;
		p = q + oldlen;
	}
	strcpy(r, p);

	return ret;
}