#ifndef _LARVA_H_
#define _LARVA_H_ 1

#include "core/var.h"

#define MIN_VARIABLES 100
#define MAX_VARIABLES 100000

var *vars;
unsigned long vars_count;

void larva_init();
void read_until_newline(char *, size_t *);
int larva_digest(char *, size_t);
int larva_stop(int);

#endif
