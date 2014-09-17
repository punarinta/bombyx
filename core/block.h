#ifndef _BLOCK_H_
#define _BLOCK_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    char *name;
    unsigned int pos;
//    unsigned int parent;
} block;

unsigned int block_init(unsigned int, char *);
unsigned int block_get_index(char *);
void block_delete_by_index(unsigned int);

#include "../common.h"

#endif
