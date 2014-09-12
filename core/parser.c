#include "parser.h"

var parse(char *str)
{
    var result;
    unsigned long val = atol(str);

    // will try to guess further
    result.type = VAR_DWORD;
    result.data = calloc(1, 4);
    memcpy(result.data, &val, 4);
    result.data_length = 4;

    return result;
}