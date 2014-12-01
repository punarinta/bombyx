#ifndef _COCOON_TEXT_H_
#define _COCOON_TEXT_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../common.h"

#ifdef __cplusplus
extern "C" {
#endif

var version_();

#ifdef __cplusplus
}
#endif

static size_t cp_strlen_utf8(const char *);

#endif
