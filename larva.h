#ifndef _LARVA_H_
#define _LARVA_H_ 1

void larva_init();
size_t read_until_token(char *, size_t *, char);
size_t read_until_not_token(char *, size_t *, char);
int larva_digest(char *, size_t);
int larva_stop(int);

#endif
