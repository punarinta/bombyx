#include "block.h"

unsigned int block_init(unsigned int pos, char *name, unsigned int parent)
{
	unsigned int i;

	// dummy block?
    if (!pos) return 0;

    // find first empty
    for (i = 1; i < blocks_count; i++) if (!blocks[i].pos) goto OK;

    larva_stop(ERR_NO_MEMORY);

    OK:;

    // save block name
    size_t len = strlen(name) + 1;
    
    blocks[i].name = calloc(len, sizeof(char));

    if (!blocks[i].name)
    {
        larva_stop(ERR_NO_MEMORY);
    }

    strcpy(blocks[i].name, name);
    blocks[i].pos = pos;
    blocks[i].parent = parent;

    return i;
}

unsigned int block_get_index(char *name)
{
    for (unsigned int i = 1; i < blocks_count; i++)
    {
        if (blocks[i].pos != VAR_UNSET && !strcmp(blocks[i].name, name))
        {
            return i;
        }
    }

    return 0;
}

void block_delete_by_index(unsigned int index)
{
    if (index && index < blocks_count)
    {
        if (blocks[index].name) { free(blocks[index].name); blocks[index].name = NULL; }
        blocks[index].pos = 0;
        blocks[index].parent = 0;
    }
}
