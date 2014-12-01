#ifndef _BOMBYX_COCOON_H_
#define _BOMBYX_COCOON_H_ 1

typedef struct _cocoon_t_
{
    char *name;
    struct _cocoon_t_ *next;
    void *ptr;
} cocoon_t;

typedef struct _cocoon_table_t_
{
    unsigned int size;
    cocoon_t **table;
} cocoon_table_t;

#endif
