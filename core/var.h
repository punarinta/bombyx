#ifndef _VAR_H_
#define _VAR_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VAR_UNSET       00
#define VAR_BYTE        01
#define VAR_WORD        02      // 16 bit
#define VAR_DWORD       03      // 32 bit
#define VAR_QWORD       04      // 64 bit
#define VAR_FLOAT       05      // 32 bit
#define VAR_DOUBLE      06      // 64 bit
#define VAR_STRING      10
#define VAR_FUNCTION    11
#define VAR_ARRAY       12
#define VAR_OBJECT      13

typedef struct
{
    unsigned short type;
    char *name;
    char *data;
    unsigned long data_size;
} var;

unsigned long var_init(char *, unsigned short, void *);
unsigned long var_get_index(char *);
int var_set_by_index(unsigned long, var, int);
void var_delete(char *);
void var_delete_by_index(unsigned long);
char *trim(char *);

var var_as_double(double);
var var_as_string(char *);
var var_add(var, var);
var var_subtract(var, var);
var var_multiply(var, var);
var var_divide(var, var);
var var_invert(var);
double var_to_double(var a);
int var_is_more(var a, var b);
int var_is_less(var a, var b);
int var_is_more_equal(var a, var b);
int var_is_less_equal(var a, var b);

#include "../common.h"

#endif
