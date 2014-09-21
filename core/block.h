#ifndef _BOMBYX_BLOCK_H_
#define _BOMBYX_BLOCK_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _block_t_
{
    char *name;
    struct _block_t_ *next;
    struct _block_t_ *parent;
    unsigned int pos;
} block_t;

typedef struct _block_table_t_
{
    unsigned int size;
    block_t **table;
} block_table_t;

block_table_t *block_table_create(int);
unsigned int block_hash(block_table_t *, char *);
block_t *block_lookup(block_table_t *, char *);
block_t *block_add(block_table_t *, char *, unsigned int, block_t *);
int block_delete(block_table_t *, char *);
void block_table_delete(block_table_t *);

// ideas: http://m.sparknotes.com/cs/searching/hashtables/problems_2.html

#endif
