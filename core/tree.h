#ifndef _BOMBYX_tree_H_
#define _BOMBYX_tree_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _tree_t_
{
    char *name;
    var v;
    struct _tree_t_ *next;
} tree_t;

typedef struct _tree_table_t_
{
    unsigned int size;
    tree_t **table;
} tree_table_t;

tree_table_t *tree_table_create(int);
unsigned int tree_hash(tree_table_t *, char *);
tree_t *tree_lookup(tree_table_t *, char *);
tree_t *tree_add(tree_table_t *, char *, unsigned int, tree_t *);
int tree_delete(tree_table_t *, char *);
void tree_table_delete(tree_table_t *);

// ideas: http://m.sparknotes.com/cs/searching/hashtables/problems_2.html

#endif
