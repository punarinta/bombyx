#ifndef _VAR_H_
#define _VAR_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VAR_UNSET       0
#define VAR_BYTE        1
#define VAR_WORD        2      // 16 bit
#define VAR_DWORD       3      // 32 bit
#define VAR_QWORD       4      // 64 bit
#define VAR_FLOAT       5      // 32 bit
#define VAR_DOUBLE      6      // 64 bit
#define VAR_STRING      7
#define VAR_BLOCK    	8
#define VAR_ARRAY       9


typedef unsigned char BYTE;
typedef unsigned int  DWORD;

char *trim(char *);

typedef struct
{
    unsigned char type;
    char *name;
    char *data;
    unsigned int data_size;
    unsigned int parent;
} var;

unsigned int var_init(char *, unsigned short, void *);
unsigned int var_get_index(char *);
BYTE var_set_by_index(unsigned int, var *);
void var_delete_by_index(unsigned int);
var var_array_element(var, unsigned int);
void var_echo(var);
void var_free(var *);

var var_as_double(double);
var var_as_string(char *);
void var_set_double(var *, double);
void var_set_string(var *, char *);

void var_sync(var);
void var_assign(var *, var *);
var var_add(var *, var *);
var var_subtract(var*, var*);
var var_multiply(var*, var*);
var var_divide(var*, var*);
var var_invert(var);
var var_increment(var);
var var_decrement(var);

double var_to_double(var *);
double var_extract_double(var);

BYTE var_is_more(var, var);
BYTE var_is_less(var, var);
BYTE var_is_more_equal(var, var);
BYTE var_is_less_equal(var, var);

#include "../common.h"

#endif
