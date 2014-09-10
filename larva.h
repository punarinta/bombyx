#ifndef _LARVA_H_
#define _LARVA_H_ 1

#include "core/var.h"

#define MIN_VARIABLES 100
#define MAX_VARIABLES 100000

var *vars;
unsigned long vars_count;

void larva_init();
size_t read_until_token(char *, size_t *, char);
size_t read_until_not_token(char *, size_t *, char);
int larva_digest(char *, size_t);
int larva_stop(int);

#endif
