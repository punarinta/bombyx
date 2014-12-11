#ifndef _BOMBYX_VAR_H_
#define _BOMBYX_VAR_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "block.h"

#define VAR_UNSET       0
#define VAR_DOUBLE      1
#define VAR_STRING      2
#define VAR_BLOCK    	4
#define VAR_ARRAY       5
#define VAR_MAP         6
#define VAR_POINTER     7
#define VAR_CUSTOM      8
#define VAR_ERROR       255

typedef unsigned char   BYTE;
typedef unsigned int    DWORD;
typedef unsigned short  uint16;

typedef struct
{
    BYTE type;              // variable type
    BYTE level;             // used to clear garbage from this level
    char *name;             // variable name
    void *data;             // pointer to the data (any object)
    DWORD data_size;        // size of memory allocated for 'data'
    struct _var_t_ *ref;    // reference to var_t that the var originated from
} var;

typedef struct _var_t_
{
    var v;
    struct _var_t_ *next;
    struct _block_t_ *parent;
} var_t;

typedef struct _var_table_t_
{
    unsigned int size;
    var_t **table;
} var_table_t;

#endif