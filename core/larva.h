#ifndef _BOMBYX_LARVA_H_
#define _BOMBYX_LARVA_H_ 1

#include "var.h"

#define RUN_IF          1
#define RUN_ELSE        2
#define RUN_WHILE       3
#define RUN_BLOCK       4

void larva_init();
void larva_chew();
void larva_digest();
void larva_silk();
void larva_error(char *);
void larva_poo();
void larva_stop();
void larva_skip_block();
void larva_read_token(char *);

#endif
