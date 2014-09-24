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

    for (; *str != '\0'; str++) hashval = *str + (hashval << 5) - hashval;

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



var *var_as_double(double a)
{
    var *v = malloc(sizeof(var));
    v->name = NULL;
    v->type = VAR_DOUBLE;
    v->data_size = sizeof(double);
    v->data = malloc(sizeof(double));
    memcpy(v->data, &a, sizeof(double));

    return v;
}

var *var_as_var_t(var_t *vt)
{
    if (!vt) return NULL;

    var *v = malloc(sizeof(var));

    v->data_size = vt->data_size;
    v->type = vt->type;

    if (vt->name)
    {
        size_t len = strlen(vt->name) + 1;
        v->name = malloc(len);
        memcpy(v->name, vt->name, len);
    }
    else v->name = NULL;

    if (vt->data)
    {
        v->data = malloc(vt->data_size);
        memcpy(v->data, vt->data, vt->data_size);
    }
    else v->data = NULL;

    return v;
}

var *var_as_string(char *a)
{
    var *v = malloc(sizeof(var));
    v->name = NULL;
    v->type = VAR_STRING;

    unsigned int len = strlen(a) + 1;
    v->data = malloc(len);
    v->data_size = len;
    memcpy(v->data, a, len);

    return v;
}

void var_set_double(var *v, double a)
{
    if (!v) return;

    if (v->data) free(v->data);

    v->type = VAR_DOUBLE;
    v->data = malloc(sizeof(double));
    v->data_size = sizeof(double);
    memcpy(v->data, &a, sizeof(double));
}

void var_set_string(var *v, char *a)
{
    if (!v) return;

    if (v->data) free(v->data);

    unsigned int len = strlen(a) + 1;
    v->type = VAR_STRING;
    v->data = malloc(len);
    v->data_size = len;
    memcpy(v->data, a, len);
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
    Unsets the variable
*/
inline void var_free(var *a)
{
    if (!a) return;
    if (a->name) free(a->name);
    if (a->data) free(a->data);
    free(a);
}

void var_echo(var *a)
{
    if (!a)
    {
        fprintf(stdout, "(null)");
    }
    else switch (a->type)
    {
        case VAR_UNSET:
        fprintf(stdout, "UNSET");
        break;

        case VAR_STRING:
        if (a->data && a->data_size) fprintf(stdout, "%s", a->data);
        else fprintf(stdout, "NULL");
        break;

        case VAR_DOUBLE:
        fprintf(stdout, "%.6g", var_extract_double(a));
        break;

        default:
        if (verbose)
        {
            if (a->data) fprintf(stdout, "\nvar_echo() failed, type: %d, data: %s\n", a->type, a->data);
            else fprintf(stdout, "\nvar_echo() failed, type: %d, data is NULL\n", a->type);
        }
        break;
    }
}
