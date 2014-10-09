#include "tree.h"
#include "sys.h"

tree_table_t *tree_table_create(int size)
{
    unsigned int i;
    tree_table_t *new_table;
    
    if (size < 1) return NULL;

    if ((new_table = malloc(sizeof(tree_table_t))) == NULL)
    {
        return NULL;
    }
    
    if ((new_table->table = malloc(sizeof(tree_t *) * size)) == NULL)
    {
        return NULL;
    }

    for (i = 0; i < size; i++) new_table->table[i] = NULL;

    new_table->size = size;

    return new_table;
}

unsigned int tree_hash(tree_table_t *hashtable, char *str)
{
    unsigned int hashval = 0;

    for (; *str != '\0'; str++) hashval = *str + (hashval << 5) - hashval;

    return hashval % hashtable->size;
}

tree_t *tree_lookup(tree_table_t *hashtable, char *str)
{
    tree_t *list;
    unsigned int hashval = tree_hash(hashtable, str);

    for (list = hashtable->table[hashval]; list != NULL; list = list->next)
    {
        if (strcmp(str, list->name) == 0) return list;
    }

    return NULL;
}

tree_t *tree_add(tree_table_t *hashtable, char *str, unsigned int pos, tree_t *parent)
{
    tree_t *new_list;
    tree_t *current_list;
    unsigned int hashval = tree_hash(hashtable, str);

    if ((new_list = malloc(sizeof(tree_t))) == NULL) return NULL;

    current_list = tree_lookup(hashtable, str);

    /* item already exists, dont insert it again. */
    if (current_list != NULL) return NULL;

    /* Insert into list */
    new_list->name = strdup(str);
    new_list->pos = pos;
    new_list->parent = parent;
    new_list->next = hashtable->table[hashval];
    hashtable->table[hashval] = new_list;

    return new_list;
}

int tree_delete(tree_table_t *hashtable, char *str)
{
    int i;
    tree_t *list, *prev;
    unsigned int hashval = tree_hash(hashtable, str);

    /* find the string in the table keeping track of the list item
     * that points to it
     */
    for (prev = NULL, list = hashtable->table[hashval];
        list != NULL && strcmp(str, list->name);
        prev = list,
        list = list->next);
    
    /* if it wasn't found, return 1 as an error */
    if (list == NULL) return 1;

    /* otherwise, it exists. remove it from the table */
    if (prev == NULL) hashtable->table[hashval] = list->next;
    else prev->next = list->next; 
    
    free(list->name);
    free(list);

    return 0;
}

void tree_table_delete(tree_table_t *hashtable)
{
    tree_t *list, *temp;

    if (hashtable == NULL) return;

    for (unsigned int i = 0; i < hashtable->size; i++)
    {
        list = hashtable->table[i];
        while (list != NULL)
        {
            temp = list;
            list = list->next;
            free(temp->name);
            free(temp);
        }
    }

    free(hashtable->table);
    free(hashtable);
}
