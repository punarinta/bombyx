#ifndef _BOMBYX_LARVA_H_
#define _BOMBYX_LARVA_H_ 1

#include "core/var.h"

void larva_init();
int larva_digest_start();
void larva_map_blocks();
var *larva_digest();
void larva_grow(unsigned long);
void larva_error();
void larva_poo();

void read_token(char *);
void skip_block();

#endif
