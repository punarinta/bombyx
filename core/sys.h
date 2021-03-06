#ifndef _BOMBYX_SYS_H_
#define _BOMBYX_SYS_H_ 1

char *strdup(const char *);
char *trim(char *);
void *bytecopy(void * const, void const * const, size_t);
double get_microtime();
char *get_file_contents(char *);
char *str_replace(const char *, const char *, const char *);
char *strtok_r(char *, const char *, char **);

#endif
