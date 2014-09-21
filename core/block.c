#include "block.h"

block_table_t *block_table_create(int size)
{
    unsigned int i;
    block_table_t *new_table;
    
    if (size < 1) return NULL; /* invalid size for table */

    /* Attempt to allocate memory for the table structure */
    if ((new_table = malloc(sizeof(block_table_t))) == NULL)
    {
        return NULL;
    }
    
    /* Attempt to allocate memory for the table itself */
    if ((new_table->table = malloc(sizeof(block_t *) * size)) == NULL)
    {
        return NULL;
    }

    /* Initialize the elements of the table */
    for (i = 0; i < size; i++) new_table->table[i] = NULL;

    /* Set the table's size */
    new_table->size = size;

    return new_table;
}

unsigned int block_hash(block_table_t *hashtable, char *str)
{
    /* we start our hash out at 0 */
    unsigned int hashval = 0;

    /* for each character, we multiply the old hash by 31 and add the current
     * character.  Remember that shifting a number left is equivalent to 
     * multiplying it by 2 raised to the number of places shifted.  So we 
     * are in effect multiplying hashval by 32 and then subtracting hashval.  
     * Why do we do this?  Because shifting and subtraction are much more 
     * efficient operations than multiplication.
     */
    for (; *str != '\0'; str++) hashval = *str + (hashval << 5) - hashval;

    /* we then return the hash value mod the hashtable size so that it will
     * fit into the necessary range
     */
    return hashval % hashtable->size;
}

block_t *block_lookup(block_table_t *hashtable, char *str)
{
    block_t *list;
    unsigned int hashval = block_hash(hashtable, str);

    /* Go to the correct list based on the hash value and see if str is
     * in the list.  If it is, return return a pointer to the list element.
     * If it isnt, the item isn't in the table, so return NULL.
     */
    for(list = hashtable->table[hashval]; list != NULL; list = list->next)
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

    /* Attempt to allocate memory for list */
    if ((new_list = malloc(sizeof(block_t))) == NULL) return NULL;

    /* Does item already exist? */
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
    if (list == NULL) return 1; /* string does not exist in table */

    /* otherwise, it exists. remove it from the table */
    if (prev == NULL) hashtable->table[hashval] = list->next;
    else prev->next = list->next; 
    
    /* free the memory associate with it */
    free(list->name);
    free(list);

    return 0;
}

void block_table_delete(block_table_t *hashtable)
{
    block_t *list, *temp;

    if (hashtable == NULL) return;

    /* Free the memory for every item in the table, including the 
     * strings themselves.
     */
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

    /* Free the table itself */
    free(hashtable->table);
    free(hashtable);
}
