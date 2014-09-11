#include "parser.h"

var parse(char *str)
{
    var result;
    long val = 345;//atol(str);

    // will try to guess further
    result.type = VAR_GENERIC;
    result.data = malloc(sizeof(long));
    memcpy(result.data, &val, sizeof(long));
    result.data_length = sizeof(long);

    return result;
}