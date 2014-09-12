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
#define VAR_STRING      10
#define VAR_FUNCTION    11
#define VAR_ARRAY       12
#define VAR_OBJECT      13

typedef struct
{
    unsigned short type;
    char *name;
    char *data;
    unsigned long data_length;
} var;

unsigned long var_add(char *, unsigned short, void *);
unsigned long var_get_index(char *);
int var_set_by_index(unsigned long, var, int);
void var_delete(char *);
void var_delete_by_index(unsigned long);
char *trim(char *);

#include "../common.h"

#endif
