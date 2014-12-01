#ifndef _BOMBYX_VAR_2_H_
#define _BOMBYX_VAR_2_H_ 1

#include "var.h"
#include "common.h"
#include "../vendor/jansson.h"

var_table_t *var_table_create(int);
unsigned int var_hash(var_table_t *, char *);
var_t *var_lookup(var_table_t *, char *);
var_t *var_add(var_table_t *, char *, BYTE, block_t *);
int var_delete(var_table_t *, char *);
void var_table_delete(bombyx_env_t *, var_table_t *);

void var_echo(bombyx_env_t *, var *);
void var_unset(bombyx_env_t *, var *);

var var_as_double(bombyx_env_t *env, double);
var var_as_string(char *, size_t);
var var_from_json(bombyx_env_t *env, char *);
var var_as_var_t(bombyx_env_t *env, var_t *);

void var_sync(bombyx_env_t *, var *);
void op_copy(bombyx_env_t *env, var *, var *);
void op_add(bombyx_env_t *, var *, var *);
void op_subtract(bombyx_env_t *, var *, var *);
void op_multiply(bombyx_env_t *, var *, var *);
void op_divide(bombyx_env_t *, var *, var *);
void op_invert(bombyx_env_t *, var *);
void op_unary_minus(bombyx_env_t *, var *);
void op_increment(bombyx_env_t *, var *);
void op_decrement(bombyx_env_t *, var *);
void op_and(bombyx_env_t *, var *, var *);
void op_or(bombyx_env_t *, var *, var *);
void op_swap(var *, var *);

BYTE var_is_true(bombyx_env_t *, var *);
BYTE var_cmp(bombyx_env_t *, var *, var *);
BYTE var_is_more(bombyx_env_t *, var *, var *);
BYTE var_is_less(bombyx_env_t *, var *, var *);
BYTE var_is_more_equal(bombyx_env_t *, var *, var *);
BYTE var_is_less_equal(bombyx_env_t *, var *, var *);

#endif