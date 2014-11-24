#include "common.h"

void larva_error(char *err)
{
    // TODO
}

var cocoon_error(FCGX_Request *request, char *err)
{
    var v = {0};
    v.type = VAR_ERROR;
    v.data = strdup(err);
    v.data_size = strlen(err) + 1;
    return v;
}