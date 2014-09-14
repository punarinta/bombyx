#ifndef _LARVA_H_
#define _LARVA_H_ 1

void larva_init();
int larva_digest();
void larva_grow(unsigned long);
void larva_error();
void larva_poo();

void read_token(char *);
void skip_block();

#endif
