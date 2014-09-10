#ifndef _VAR_H_
#define _VAR_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VAR_GENERIC     01
#define VAR_INTEGER     02
#define VAR_STRING      03
#define VAR_FUNCTION    10
#define VAR_ARRAY       11
#define VAR_OBJECT      12

typedef struct
{
    unsigned short int type;
    char *name;
    void *data;
} var;

size_t var_add(char *, unsigned short, void *);
void var_delete(char *);
void var_delete_by_index(unsigned long);

#endif
