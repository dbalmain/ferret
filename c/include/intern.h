#ifndef _SYMBOL_H
#define _SYMBOL_H

extern void frt_intern_init();
extern const char *frt_intern(const char *str);
extern const char *frt_intern_and_free(char *str);

#define FRT_I frt_intern
#define FRT_IF frt_intern_and_free

#endif
