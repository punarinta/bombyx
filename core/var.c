#include "var.h"
#include "sys.h"
#include "../common.h"

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
        if (strcmp(str, list->name) == 0) return list;
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
    new_list->data = NULL;
    new_list->data_size = 0;
    new_list->name = strdup(str);
    new_list->type = type;
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
        list != NULL && strcmp(str, list->name);
        prev = list,
        list = list->next);
    
    /* if it wasn't found, return 1 as an error */
    if (list == NULL) return 1;

    /* otherwise, it exists. remove it from the table */
    if (prev == NULL) hashtable->table[hashval] = list->next;
    else prev->next = list->next; 
    
    if (list->name) free(list->name);
    if (list->data) free(list->data);
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
            if (temp->name) free(temp->name);
            if (temp->data) free(temp->data);
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
    var_t *v = var_lookup(vars, a->name);
    if (v)
    {
        if (v->data_size == a->data_size && v->type == a->type)
        {
            memcpy(v->data, a->data, a->data_size);
        }
        else
        {
            v->type = a->type;
            v->data_size = a->data_size;

            if (v->data)
            {
                free(v->data);
            }

            v->data = malloc(a->data_size);
            memcpy(v->data, a->data, a->data_size);
        }
    }
    else
    {
        sprintf(temp_error, "Variable '%s' not found.", a->name);
        larva_error(temp_error);
    }
}

var var_as_double(double a)
{
    var v = {0};
    v.type = VAR_DOUBLE;
    v.data_size = sizeof(double);
    v.data = malloc(sizeof(double));
    memcpy(v.data, &a, sizeof(double));

    return v;
}

/*
    Be careful: name is not copied, but assigned!
*/
var var_as_var_t(var_t *vt)
{
    var v = {0};

    v.data_size = vt->data_size;
    v.type = vt->type;
    v.name = vt->name;

    if (vt->data)
    {
        v.data = malloc(vt->data_size);
        memcpy(v.data, vt->data, vt->data_size);
    }
    else v.data = NULL;

    return v;
}

var var_as_string(char *a)
{
    var v = {0};
    v.type = VAR_STRING;

    unsigned int len = strlen(a) + 1;
    v.data = malloc(len);
    v.data_size = len;
    memcpy(v.data, a, len);

    return v;
}

void var_set_double(var *v, double a)
{
    larva_error("var_set_double() is obsolete");
/*    if (!v) return;

    if (v->data) free(v->data);

    v->type = VAR_DOUBLE;
    v->data = malloc(sizeof(double));
    v->data_size = sizeof(double);
    memcpy(v->data, &a, sizeof(double));*/
}

/*
    Destroys the variable
    Returns it's double
*/
double var_to_double(var *a)
{
    double d;

    if (!a || !a->data) return 0;

    if (a->type == VAR_DOUBLE) memcpy(&d, a->data, sizeof(double));
    else if (a->type == VAR_STRING) d = atof(a->data);
    else d = 0;

    var_free(a);

    return d;
}

double var_extract_double(var *a)
{
    double d;

    if (!a || !a->data) return 0;

    if (a->type == VAR_DOUBLE) memcpy(&d, a->data, sizeof(double));
    else if (a->type == VAR_STRING) d = atof(a->data);
    else d = 0;

    return d;
}

/*
    Frees variable memory
*/
inline void var_free(var *a)
{
    if (!a) return;
    if (a->name) free(a->name);
    if (a->data) free(a->data);
    free(a);
}

inline void var_unset(var *a)
{
    if (a->data) free(a->data);
}

/*inline var var_unset_ret(var a)
{
    if (a.name) free(a.name);
    if (a.data) free(a.data);
    a.name = NULL;
    a.data = NULL;
    return a;
}*/

void var_echo(var *a)
{
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
            fprintf(stdout, "%.6g", var_extract_double(a));
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
