#ifndef _LARVA_H_
#define _LARVA_H_ 1

#include "core/var.h"

void larva_init();
int larva_digest_start();
var larva_digest();
void larva_grow(unsigned long);
void larva_error();
void larva_poo();

void read_token(char *);
void skip_block();

#endif
