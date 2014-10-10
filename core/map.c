#include "map.h"
#include "sys.h"

map_table_t *map_table_create(int size)
{
    unsigned int i;
    map_table_t *new_table;
    
    if (size < 1) return NULL;

    if ((new_table = malloc(sizeof(map_table_t))) == NULL)
    {
        return NULL;
    }
    
    if ((new_table->table = malloc(sizeof(map_t *) * size)) == NULL)
    {
        return NULL;
    }

    for (i = 0; i < size; i++) new_table->table[i] = NULL;

    new_table->size = size;

    return new_table;
}

unsigned int map_hash(map_table_t *hashtable, char *str)
{
    unsigned int hashval = 0;

    for (; *str != '\0'; str++) hashval = *str + (hashval << 5) - hashval;

    return hashval % hashtable->size;
}

map_t *map_lookup(map_table_t *hashtable, char *str)
{
    map_t *list;
    unsigned int hashval = map_hash(hashtable, str);

    for (list = hashtable->table[hashval]; list != NULL; list = list->next)
    {
        if (strcmp(str, list->name) == 0) return list;
    }

    return NULL;
}

map_t *map_add(map_table_t *hashtable, char *str, var v)
{
    map_t *new_list;
    map_t *current_list;
    unsigned int hashval = map_hash(hashtable, str);

    if ((new_list = malloc(sizeof(map_t))) == NULL) return NULL;

    current_list = map_lookup(hashtable, str);

    /* item already exists, dont insert it again. */
    if (current_list != NULL) return NULL;

    /* Insert into list */
    new_list->name = strdup(str);
    new_list->v = v;
    new_list->next = hashtable->table[hashval];
    hashtable->table[hashval] = new_list;

    return new_list;
}

int map_delete(map_table_t *hashtable, char *str)
{
    int i;
    map_t *list, *prev;
    unsigned int hashval = map_hash(hashtable, str);

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

void map_table_delete(map_table_t *hashtable)
{
    map_t *list, *temp;

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
