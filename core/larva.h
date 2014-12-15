#ifndef _BOMBYX_LARVA_H_
#define _BOMBYX_LARVA_H_ 1

#include <stdarg.h>
#include "common.h"

#define RUN_IF          1
#define RUN_ELSE        2
#define RUN_WHILE       3
#define RUN_BLOCK       4

void larva_init(bombyx_env_t *, char *, size_t);
char *larva_chew(bombyx_env_t *, char *, size_t, size_t*);
void larva_digest(bombyx_env_t *);
void larva_silk(bombyx_env_t *);
void larva_error(bombyx_env_t *, char *, ...);
void runtime_error(bombyx_env_t *, char *, ...);
void larva_poo(bombyx_env_t *);
void larva_stop(bombyx_env_t *);
void larva_skip_block(bombyx_env_t *);
void larva_read_token(bombyx_env_t *, char *);
void larva_read_string_token(bombyx_env_t *, char *);

void stack_push(bombyx_env_t *, var);
var stack_pop(bombyx_env_t *);
void stack_clear(bombyx_env_t *);
void stack_poo(bombyx_env_t *);

#endif
