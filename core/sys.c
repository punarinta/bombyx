#include "../common.h"

char* itoa(int value, char* result, int base)
{
    // check that the base if valid
    if (base < 2 || base > 36)
    {
        *result = '\0';
        return result;
    }

    int tmp_value;
    char *ptr = result, *ptr1 = result, tmp_char;

    do
    {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while (value);

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';

    while(ptr1 < ptr)
    {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }

    return result;
}

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