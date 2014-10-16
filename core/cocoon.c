#include "../common.h"
#include "sys.h"
#include "larva.h"

cocoon_table_t *cocoon_table_create(int size)
{
    unsigned int i;
    cocoon_table_t *new_table;
    
    if (size < 1) return NULL;

    if ((new_table = malloc(sizeof(cocoon_table_t))) == NULL)
    {
        return NULL;
    }
    
    if ((new_table->table = malloc(sizeof(cocoon_t *) * size)) == NULL)
    {
        return NULL;
    }

    for (i = 0; i < size; i++) new_table->table[i] = NULL;

    new_table->size = size;

    return new_table;
}

unsigned int cocoon_hash(cocoon_table_t *hashtable, char *str)
{
    unsigned int hashval = 0;

    for (; *str != '\0'; str++) hashval = *str + (hashval << 5) - hashval;

    return hashval % hashtable->size;
}

cocoon_t *cocoon_lookup(cocoon_table_t *hashtable, char *str)
{
    cocoon_t *list;
    unsigned int hashval = cocoon_hash(hashtable, str);

    for (list = hashtable->table[hashval]; list != NULL; list = list->next)
    {
        if (strcmp(str, list->name) == 0) return list;
    }

    return NULL;
}

cocoon_t *cocoon_add(cocoon_table_t *hashtable, char *cocoon_name)
{
    cocoon_t *new_list;
    cocoon_t *current_list;

    unsigned int hashval = cocoon_hash(hashtable, cocoon_name);

    if ((new_list = malloc(sizeof(cocoon_t))) == NULL) return NULL;

    current_list = cocoon_lookup(hashtable, cocoon_name);

    /* item already exists, dont insert it again. */
    if (current_list != NULL) return NULL;

    // check the cocoon
    // TODO: search in the lair directory first
    void *lib_handle = dlopen(cocoon_name, RTLD_LAZY);

    if (!lib_handle)
    {
        fprintf(stderr, "Cannot load cocoon '%s'.\n", cocoon_name);
        larva_error(0);
    }

    /* Insert into list */
    new_list->name = strdup(cocoon_name);
    new_list->ptr = lib_handle;
    new_list->next = hashtable->table[hashval];
    hashtable->table[hashval] = new_list;

    return new_list;
}

void cocoon_table_delete(cocoon_table_t *hashtable)
{
    cocoon_t *list, *temp;

    if (hashtable == NULL) return;

    for (unsigned int i = 0; i < hashtable->size; i++)
    {
        list = hashtable->table[i];
        while (list != NULL)
        {
            temp = list;
            list = list->next;
            dlclose(temp->ptr);
            free(temp->name);
            free(temp);
        }
    }

    free(hashtable->table);
    free(hashtable);
}
