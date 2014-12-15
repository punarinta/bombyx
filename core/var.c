#include "var.h"
#include "var_2.h"
#include "map_2.h"
#include "array_2.h"
#include "map.h"
#include "array.h"
#include "sys.h"
#include "bytecode.h"
#include "common.h"
#include "larva.h"

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

/*
    A bit faster than var_lookup(), but requires string length
*/
var_t *var_lookup_2(var_table_t *hashtable, char *str, size_t strlen)
{
    var_t *list;
    unsigned int hashval = var_hash(hashtable, str);

    for (list = hashtable->table[hashval]; list != NULL; list = list->next)
    {
        if (memcmp(str, list->v.name, strlen) == 0) return list;
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
    if (current_list != NULL) return current_list; // NULL;

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

void var_table_delete(bombyx_env_t *env, var_table_t *hashtable, int reset_only)
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
            var_unset(env, &temp->v);
            free(temp);
        }
    }

    if (reset_only)
    {
        memset(hashtable->table, 0, sizeof(var_t *) * hashtable->size);
    }
    else
    {
        free(hashtable->table);
        free(hashtable);
    }
}


/*
    Synchronizes var with vars[] if necessary
    Var is NOT modified
*/
void var_sync(bombyx_env_t *env, var *a)
{
    if (a->ref)
    {
        op_copy(env, &(a->ref)->v, a);
    }
    else if (a->name)
    {
        var_t *vt = var_lookup(env->vars, a->name);
        if (vt)
        {
            op_copy(env, &vt->v, a);
        }
        else
        {
            runtime_error(env, "Variable '%s' not found.", a->name);
        }
    }
}

var var_as_double(bombyx_env_t *env, double a)
{
    var v = {0};
    v.type = VAR_DOUBLE;
    v.data_size = sizeof(double);
    v.data = challoc(env->pool_of_doubles);
    *(double *)v.data = a;

    return v;
}

/*
    Be careful: name is not copied, but assigned!
*/
var var_as_var_t(bombyx_env_t *env, var_t *vt)
{
    var v = {0};
    v.name = vt->v.name;

    op_copy(env, &v, &vt->v);
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

var var_from_json(bombyx_env_t *env, char *a)
{
    var v = {0};

    json_error_t error;
    json_t *j = json_loads(a, 0, &error);  // JSON_DECODE_ANY
    if (!j)
    {
        runtime_error(env, "Cannot decode JSON variable '%s'.", a);
    }

    if (json_is_object(j))
    {
        v.type = VAR_MAP;
        v.data = json_to_map(env, j);
        v.data_size = sizeof(map_table_t);
    }
    else if (json_is_array(j))
    {
        v.type = VAR_ARRAY;
        v.data = json_to_array(env, j);
        v.data_size = sizeof(array_t);
    }
    else
    {
        // this can be only array or object
        runtime_error(env, "JSON parsing error");
    }

    json_decref(j);

    return v;
}

/*
    Returns a pointer to a variable object pointed by APath
*/
var *var_apath(bombyx_env_t *env, void *pointer, char *apath)
{
    var v = {0};
    var *object = pointer;

    int index;
    char *saveptr, *tok = strtok_r(apath, ".", &saveptr);

    while (tok && pointer)
    {
        index = atoi(tok);

        if (index || tok[0] == '0')
        {
            // it's a number -> array access
            if (object->type != VAR_ARRAY)
            {
                return NULL;
            }

            object = ((array_t *)object->data)->vars[index];
        }
        else
        {
            if (object->type != VAR_MAP)
            {
                return NULL;
            }

            map_t *m = map_lookup(object->data, tok);
            object = &m->v;
        }

        tok = strtok_r(NULL, ".", &saveptr);
        if (!tok) break;
    }

    return object;
}

map_table_t *json_to_map(bombyx_env_t *env, json_t *json)
{
    size_t index;
    json_t *value;
    const char *key;
    map_table_t *map;

    // TODO: minimum map size (100?)

    if (json_is_object(json))
    {
        map = map_table_create(json_object_size(json));
        json_object_foreach(json, key, value)
        {
            var v = {0};
            if (json_is_object(value))
            {
                // process recursively
                v.type = VAR_MAP;
                v.data = json_to_map(env, value);
                v.data_size = sizeof(map_table_t);
            }
            else if (json_is_array(value))
            {
                v.type = VAR_ARRAY;
                v.data = json_to_array(env, value);
                v.data_size = sizeof(array_t);
            }
            else if (json_is_string(value))
            {
                v.type = VAR_STRING;
                v.data = strdup(json_string_value(value));
                v.data_size = json_string_length(value) + 1;
            }
            else if (json_is_number(value))
            {
                v.type = VAR_DOUBLE;
                v.data = challoc(env->pool_of_doubles);
                v.data_size = sizeof(double);
                *(double *)v.data = json_number_value(value);
            }
            map_add(env, map, (char *)key, v);
        }
    }

    return map;
}

array_t *json_to_array(bombyx_env_t *env, json_t *json)
{
    size_t index;
    json_t *value;
    const char *key;
    array_t *array;

    if (json_is_array(json))
    {
        array = array_create(json_array_size(json));
        json_array_foreach(json, index, value)
        {
            var v = {0};
            if (json_is_object(value))
            {
                // process recursively
                v.type = VAR_MAP;
                v.data = json_to_map(env, value);
                v.data_size = sizeof(map_table_t);
            }
            else if (json_is_array(value))
            {
                v.type = VAR_ARRAY;
                v.data = json_to_array(env, value);
                v.data_size = sizeof(array_t);
            }
            else if (json_is_string(value))
            {
                v.type = VAR_STRING;
                v.data = strdup(json_string_value(value));
                v.data_size = json_string_length(value) + 1;
            }
            else if (json_is_number(value))
            {
                v.type = VAR_DOUBLE;
                v.data = challoc(env->pool_of_doubles);
                v.data_size = sizeof(double);
                *(double *)v.data = json_number_value(value);
            }
            array_push(array, v);
        }
    }

    return array;
}

void var_unset(bombyx_env_t *env, var *a)
{
    if (a->data)
    {
        if (a->type == VAR_DOUBLE) chfree(env->pool_of_doubles, a->data);
        else if (a->type == VAR_MAP) map_table_delete(env, a->data);
        else if (a->type == VAR_ARRAY) array_delete(env, a->data);
        else if (a->type == VAR_POINTER) return;
        else free(a->data);
    }
}

BYTE var_echo_level = 0;

void var_echo(bombyx_env_t *env, var *a)
{
    char *str;
    map_t *list;
    map_table_t *ht;
    ++var_echo_level;

    // in web version the first echo starts the body (no more headers will be allowed)
    if (env->wd && !env->wd->body_started)
    {
        env->wd->body_started = 1;
        web_puts(env, "\r\n");
    }

    if (a)
    {
        switch (a->type)
        {
            case VAR_UNSET:
            web_puts(env, "UNSET");
            break;

            case VAR_STRING:
            if (a->data && a->data_size) web_printf(env, a->data, 0);
            else web_printf(env, "NULL%c", 0);
            break;

            case VAR_DOUBLE:
            web_printf(env, "%.6g", *(double *)a->data);
            break;

            case VAR_MAP:
            fprintf(stdout, "\n%.*s{\n", var_echo_level - 1, "\t\t\t\t\t");
            for (unsigned int i = 0; i < ((map_table_t *)a->data)->size; i++)
            {
                list = ((map_table_t *)a->data)->table[i];
                while (list != NULL)
                {
                    if (list->v.type)
                    {
                        fprintf(stdout, "%.*s\"%s\" : ", var_echo_level, "\t\t\t\t\t", list->name);
                        var_echo(env, &list->v);
                        fprintf(stdout, "\n");
                    }
                    list = list->next;
                }
            }
            fprintf(stdout, "%.*s}", var_echo_level - 1, "\t\t\t\t\t");
            break;

            case VAR_ARRAY:
            fprintf(stdout, "\n%.*s[\n", var_echo_level - 1, "\t\t\t\t\t");
            for (unsigned int i = 0; i < ((array_t *)a->data)->size; i++)
            {
                if (((array_t *)a->data)->vars[i] && ((array_t *)a->data)->vars[i]->type)
                {
                    fprintf(stdout, "%.*s%d: ", var_echo_level, "\t\t\t\t\t", i);
                    var_echo(env, ((array_t *)a->data)->vars[i]);
                    fprintf(stdout, "\n");
                }
            }
            fprintf(stdout, "%.*s]", var_echo_level - 1, "\t\t\t\t\t");
            break;

            default:
            if (verbose)
            {
                fprintf(stdout, "\ntype = %d, data = ", a->type);
                if (a->data)
                {
                    // puts(a->data);
                    fwrite(a->data, a->data_size, 1, stdout);
                    fflush(stdout);
                }
                else puts("NULL");
            }
            break;
        }
    }
    else
    {
        web_puts(env, "(null)");
    }
    --var_echo_level;
}
