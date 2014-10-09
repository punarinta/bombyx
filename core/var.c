#include "var.h"
#include "sys.h"
#include "bytecode.h"
#include "../common.h"
#include "../vendor/jansson.h"

var_table_t *var_table_create(int size)
{
    unsigned int i;
    var_table_t *new_table;
    
    if (size < 1) return NULL;

    if ((new_table = malloc(sizeof(var_table_t))) == NULL)
    {
        return NULL;
    }
    
    if ((new_table->table = malloc(sizeof(var_t *) * size)) == NULL)
    {
        return NULL;
    }

    for (i = 0; i < size; i++) new_table->table[i] = NULL;

    new_table->size = size;

    return new_table;
}

unsigned int var_hash(var_table_t *hashtable, char *str)
{
    unsigned int hashval = 0;

    for (; *str != '\0'; ++str) hashval = *str + (hashval << 5) - hashval;

    return hashval % hashtable->size;
}

var_t *var_lookup(var_table_t *hashtable, char *str)
{
    var_t *list;
    unsigned int hashval = var_hash(hashtable, str);

    for (list = hashtable->table[hashval]; list != NULL; list = list->next)
    {
        if (strcmp(str, list->v.name) == 0) return list;
    }

    return NULL;
}

var_t *var_add(var_table_t *hashtable, char *str, BYTE type, block_t *parent)
{
    var_t *new_list;
    var_t *current_list;
    unsigned int hashval = var_hash(hashtable, str);

    if ((new_list = malloc(sizeof(var_t))) == NULL) return NULL;

    current_list = var_lookup(hashtable, str);

    /* item already exists, dont insert it again. */
    if (current_list != NULL) return NULL;

    /* Insert into list */
    new_list->v.data = NULL;
    new_list->v.data_size = 0;
    new_list->v.name = strdup(str);
    new_list->v.type = type;
    // TODO: figure out shit with level
    new_list->v.level = 0;
    new_list->parent = parent;
    new_list->next = hashtable->table[hashval];
    hashtable->table[hashval] = new_list;

    return new_list;
}

int var_delete(var_table_t *hashtable, char *str)
{
    int i;
    var_t *list, *prev;
    unsigned int hashval = var_hash(hashtable, str);

    /* find the string in the table keeping track of the list item
     * that points to it
     */
    for (prev = NULL, list = hashtable->table[hashval];
        list != NULL && strcmp(str, list->v.name);
        prev = list,
        list = list->next);
    
    /* if it wasn't found, return 1 as an error */
    if (list == NULL) return 1;

    /* otherwise, it exists. remove it from the table */
    if (prev == NULL) hashtable->table[hashval] = list->next;
    else prev->next = list->next; 
    
    if (list->v.name) free(list->v.name);
    if (list->v.data) free(list->v.data);
    free(list);

    return 0;
}

void var_table_delete(var_table_t *hashtable)
{
    var_t *list, *temp;

    if (hashtable == NULL) return;

    for (unsigned int i = 0; i < hashtable->size; i++)
    {
        list = hashtable->table[i];
        while (list != NULL)
        {
            temp = list;
            list = list->next;
            if (temp->v.name) free(temp->v.name);
            if (temp->v.data && temp->v.type != VAR_DOUBLE) free(temp->v.data);
            free(temp);
        }
    }

    free(hashtable->table);
    free(hashtable);
}


/*
    Synchronizes var with vars[] if necessary
    Var is NOT modified
*/
void var_sync(var *a)
{
    if (a->ref)
    {
        op_copy(&((var_t *)a->ref)->v, a);
    }
    else if (a->name)
    {
        var_t *vt = var_lookup(vars, a->name);
        if (vt)
        {
            op_copy(&vt->v, a);
        }
        else
        {
            sprintf(temp_error, "Variable '%s' not found.", a->name);
            larva_error(temp_error);
        }
    }
}

var var_as_double(double a)
{
    var v = {0};
    v.type = VAR_DOUBLE;
    v.data_size = sizeof(double);
    v.data = challoc(pool_of_doubles);
    *(double *)v.data = a;

    return v;
}

/*
    Be careful: name is not copied, but assigned!
*/
var var_as_var_t(var_t *vt)
{
    var v = {0};
    v.name = vt->v.name;

    op_copy(&v, &vt->v);
    v.ref = vt;

    return v;
}

var var_as_string(char *a, size_t len)
{
    var v = {0};
    v.type = VAR_STRING;
    v.data = malloc(++len);
    v.data_size = len;
    memcpy(v.data, a, len);

    return v;
}

var var_as_json(char *a)
{
    var v = {0};
    v.type = VAR_JSON;
    json_error_t error;
    json_t *jt = json_loads(a, 0, &error);  // JSON_DECODE_ANY
    if (!jt)
    {
        sprintf(temp_error, "Cannot decode JSON variable '%s'.", a);
        larva_error(temp_error);
    }

    v.data_size = sizeof(json_t);
    v.data = jt;

    return v;
}

inline void var_unset(var *a)
{
    if (a->data)
    {
        if (a->type == VAR_DOUBLE) chfree(pool_of_doubles, a->data);
        else if (a->type == VAR_JSON) json_decref(a->data);
        else free(a->data);
    }
}

void var_echo(var *a)
{
    char *str;
    if (a)
    {
        switch (a->type)
        {
            case VAR_UNSET:
            fputs("UNSET", stdout);
            break;

            case VAR_STRING:
            if (a->data && a->data_size) fputs(a->data, stdout);
            else fputs("NULL", stdout);
            break;

            case VAR_DOUBLE:
            fprintf(stdout, "%.6g", *(double *)a->data);
            break;

            case VAR_JSON:
            str = json_dumps((json_t *)a->data, 0);
            fprintf(stdout, "%s", str);
            free(str);
            break;

            default:
            if (verbose)
            {
                fprintf(stdout, "\nvar_echo() failed, type = %d, data = ", a->type);
                if (a->data) puts(a->data);
                else puts("NULL");
            }
            break;
        }
    }
    else
    {
        fputs("(null)", stdout);
    }
}
