#include "block.h"
#include "sys.h"

block_table_t *block_table_create(int size)
{
    unsigned int i;
    block_table_t *new_table;
    
    if (size < 1) return NULL;

    if ((new_table = malloc(sizeof(block_table_t))) == NULL)
    {
        return NULL;
    }
    
    if ((new_table->table = malloc(sizeof(block_t *) * size)) == NULL)
    {
        return NULL;
    }

    for (i = 0; i < size; i++) new_table->table[i] = NULL;

    new_table->size = size;

    return new_table;
}

unsigned int block_hash(block_table_t *hashtable, char *str)
{
    unsigned int hashval = 0;

    for (; *str != '\0'; str++) hashval = *str + (hashval << 5) - hashval;

    return hashval % hashtable->size;
}

block_t *block_lookup(block_table_t *hashtable, char *str)
{
    block_t *list;
    unsigned int hashval = block_hash(hashtable, str);

    for (list = hashtable->table[hashval]; list != NULL; list = list->next)
    {
        if (strcmp(str, list->name) == 0) return list;
    }

    return NULL;
}

block_t *block_add(block_table_t *hashtable, char *str, unsigned int pos, block_t *parent)
{
    block_t *new_list;
    block_t *current_list;
    unsigned int hashval = block_hash(hashtable, str);

    if ((new_list = malloc(sizeof(block_t))) == NULL) return NULL;

    current_list = block_lookup(hashtable, str);

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

int block_delete(block_table_t *hashtable, char *str)
{
    int i;
    block_t *list, *prev;
    unsigned int hashval = block_hash(hashtable, str);

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

void block_table_delete(block_table_t *hashtable)
{
    block_t *list, *temp;

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
