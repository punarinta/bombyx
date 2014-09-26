#ifndef _BOMBYX_LARVA_H_
#define _BOMBYX_LARVA_H_ 1

#include "core/var.h"

void larva_init();
void larva_chew();
var *larva_digest();
void larva_error(char *);
void larva_poo();
void larva_stop();
void larva_skip_block();
void larva_read_token(char *);

#endif
