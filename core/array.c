#include "array.h"
#include "sys.h"

array_table_t *array_table_create(int size)
{
    unsigned int i;
    array_table_t *new_table;
    
    if (size < 1) return NULL;

    if ((new_table = malloc(sizeof(array_table_t))) == NULL)
    {
        return NULL;
    }
    
    if ((new_table->table = malloc(sizeof(array_t *) * size)) == NULL)
    {
        return NULL;
    }

    for (i = 0; i < size; i++) new_table->table[i] = NULL;

    new_table->size = size;

    return new_table;
}

unsigned int array_hash(array_table_t *hashtable, char *str)
{
    unsigned int hashval = 0;

    for (; *str != '\0'; str++) hashval = *str + (hashval << 5) - hashval;

    return hashval % hashtable->size;
}

array_t *array_lookup(array_table_t *hashtable, char *str)
{
    array_t *list;
    unsigned int hashval = array_hash(hashtable, str);

    for (list = hashtable->table[hashval]; list != NULL; list = list->next)
    {
        if (strcmp(str, list->name) == 0) return list;
    }

    return NULL;
}

array_t *array_add(array_table_t *hashtable, char *str)
{
    array_t *new_list;
    array_t *current_list;
    unsigned int hashval = array_hash(hashtable, str);

    if ((new_list = malloc(sizeof(array_t))) == NULL) return NULL;

    current_list = array_lookup(hashtable, str);

    /* item already exists, dont insert it again. */
    if (current_list != NULL) return NULL;

    /* Insert into list */
    new_list->name = strdup(str);
    new_list->next = hashtable->table[hashval];
    hashtable->table[hashval] = new_list;

    return new_list;
}

int array_delete(array_table_t *hashtable, char *str)
{
    int i;
    array_t *list, *prev;
    unsigned int hashval = array_hash(hashtable, str);

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

void array_table_delete(array_table_t *hashtable)
{
    array_t *list, *temp;

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
