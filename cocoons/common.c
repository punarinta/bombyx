#include "common.h"

void larva_error(bombyx_env_t *env, char *err, ...)
{
    // TODO
}

void runtime_error(bombyx_env_t *env, char *err, ...)
{
    // TODO
}

var cocoon_error(bombyx_env_t *env, char *err, ...)
{
    char error_text[256];
    va_list args;
    va_start(args, err);
    vsprintf(error_text, err, args);
    web_puts(env, error_text);
    va_end(args);

    var v = {0};
    v.type = VAR_ERROR;
    v.data = strdup(error_text);
    v.data_size = strlen(error_text) + 1;
    return v;
}

void random_string(char *s, const int len)
{
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i)
    {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = 0;
}