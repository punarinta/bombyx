#ifndef _COMMON_H_
#define _COMMON_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "core/var.h"
#include "core/expression.h"

#define MIN_VARIABLES       100
#define MAX_VARIABLES       100000

#define ERR_NO_MEMORY       1
#define ERR_SYNTAX          2
#define ERR_TOO_LONG        3

var *vars;
char *gl_code;
unsigned long vars_count;

void larva_grow(unsigned long);
void larva_error(unsigned long);
int larva_stop(int);
size_t read_until_token(char *, size_t *, char);
size_t read_until_not_token(char *, size_t *, char);
char* itoa(int, char*, int);
void echo_int(int);

#endif
