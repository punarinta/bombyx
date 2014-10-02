/*
 * (C) Copyright 2011, 2012, 2013, Dario Hamidi <dario.hamidi@gmail.com>
 *
 * This file is part of Challoc.
 * 
 * Challoc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Challoc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Challoc.  If not, see <http://www.gnu.org/licenses/>.
 */ 
#include "challoc.h"
#include <stdlib.h>
#include <assert.h>

struct chunk_allocator
{
     size_t n_chunks;            /* number of chunks this allocator holds */
     size_t chunk_size;          /* size of single chunk in bytes         */
     size_t current_chunk;       /* stack pointer in chunks               */
     ChunkAllocator* next;       /* next allocator, if this one is full   */
     unsigned char** chunks;     /* stack of free locations in memory     */
     unsigned char* memory;      /* challoc returns memory from here      */
};

static ChunkAllocator* get_first_allocator_with_free_chunk(ChunkAllocator* start)
{
    ChunkAllocator* iter = NULL;

    for (iter = start; iter; iter = iter->next)
    {
        if (iter->current_chunk > 0)
        {
            return iter;
        }

        /* all allocators are full, so we append a new one and return it */
        if (iter->next == NULL)
        {
            iter->next = chcreate(iter->n_chunks, iter->chunk_size);
            return iter->next;
        }
    }

   /* never reached */
    assert(0);
    return NULL;
}

static void push_chunk_to_first_free_stack(ChunkAllocator* root, void* p)
{
    ChunkAllocator* iter = NULL;

    for (iter = root; iter; iter = iter->next)
    {
        if (iter->current_chunk < iter->n_chunks)
        {
            iter->chunks[iter->current_chunk++] = p;
            return;
        }
    }

    /* if we get here, there was no room for storing p in any allocator reachable from root */
    assert(0);
}

void* challoc(ChunkAllocator* root)
{
    // We assume that root is not NULL

    root = get_first_allocator_with_free_chunk(root);
     
    return root->chunks[--root->current_chunk];
}

void chfree(ChunkAllocator* root, void* p)
{
    // We assume that root is not NULL

    /* all memory in this allocator is free already */
    if (root->current_chunk == 0)
        push_chunk_to_first_free_stack(root, p);
    else
        root->chunks[root->current_chunk++] = p;
}

void chclear(ChunkAllocator* root)
{
    ChunkAllocator* iter = NULL;

    if (!root) return;

    for (iter = root; iter; iter = iter->next)
    {
        iter->current_chunk = iter->n_chunks;
    }
}

ChunkAllocator* chcreate(size_t n_chunks, size_t chunk_size)
{
    size_t i;
    ChunkAllocator* s = NULL;

    s = malloc(sizeof(*s));

    if (!s) goto FAIL;

    s->chunks = calloc(n_chunks, sizeof(*s->chunks));
    if (!s->chunks) goto CLEAR1;

    s->memory = calloc(n_chunks, chunk_size);
    if (!s->memory) goto CLEAR2;
     
    s->n_chunks = n_chunks;
    s->chunk_size = chunk_size;
    s->current_chunk = 0;
    s->next = NULL;
     
     /* add locations in s->memory to the stack of free chunks */
    while (s->current_chunk < s->n_chunks)
    {
        s->chunks[s->current_chunk] = &s->memory[s->current_chunk * chunk_size];
        s->current_chunk++;
    }

    return s;

CLEAR1:
    free(s);
CLEAR2:
    free(s->chunks);
FAIL:
    return NULL;
}

void chdestroy(ChunkAllocator** root)
{
    ChunkAllocator* cur = NULL;
    ChunkAllocator* next = NULL;

    for (cur = *root; cur; cur = next)
    {
        next = cur->next;

        free(cur->memory);
        free(cur->chunks);
        free(cur);
    }

    *root = NULL;
}
