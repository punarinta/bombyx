#ifndef _BOMBYX_VAR_H_
#define _BOMBYX_VAR_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "block.h"

#define VAR_UNSET       0
#define VAR_DOUBLE      1
#define VAR_STRING      2
#define VAR_BLOCK    	3
#define VAR_ARRAY       4
#define VAR_HASHTABLE   5
#define VAR_POINTER     6

typedef unsigned char BYTE;
typedef unsigned int  DWORD;

typedef struct
{
    BYTE type;
    BYTE level;         // used to clear garbage from this level
    char *name;
    char *data;
    DWORD data_size;
} var;

typedef struct _var_t_
{
    BYTE type;
    char *name;
    char *data;
    DWORD data_size;
    struct _var_t_ *next;
    struct _block_t_ *parent;
} var_t;

typedef struct _var_table_t_
{
    unsigned int size;
    var_t **table;
} var_table_t;

var_table_t *var_table_create(int);
unsigned int var_hash(var_table_t *, char *);
var_t *var_lookup(var_table_t *, char *);
var_t *var_add(var_table_t *, char *, BYTE, block_t *);
int var_delete(var_table_t *, char *);
void var_table_delete(var_table_t *);

void var_echo(var *);
void var_free(var *);
void var_unset(var *);
var var_unset_ret(var);

var var_as_double(double);
var var_as_string(char *);
var var_as_var_t(var_t *);
void var_set_double(var *, double);

void var_sync(var *);
void op_copy(var *, var *);
void op_add(var *, var *);
void op_subtract(var *, var *);
void op_multiply(var *, var *);
void op_divide(var *, var *);
void op_invert(var *);
void op_unary_minus(var *);
void op_increment(var *);
void op_decrement(var *);
void op_and(var *, var *);
void op_or(var *, var *);

double var_to_double(var *);
double var_extract_double(var *);

BYTE var_is_true(var *);
BYTE var_cmp(var *, var *);
BYTE var_is_more(var *, var *);
BYTE var_is_less(var *, var *);
BYTE var_is_more_equal(var *, var *);
BYTE var_is_less_equal(var *, var *);

#endif
