#ifndef _SYMBOL_H
#define _SYMBOL_H

typedef struct Frt__Symbol {
    void *value;
} Frt__Symbol;

typedef Frt__Symbol *FrtSymbol;

extern void frt_symbol_init();
extern FrtSymbol frt_intern(const char *str);
extern FrtSymbol frt_intern_and_free(char *str);

#define FRT_I frt_intern
#define FRT_IF frt_intern_and_free
#define FRT_S(sym) ((const char *)sym)

#define frt_sym_hash(sym) frt_str_hash((const char *)sym)
#define frt_sym_len(sym) strlen((const char *)sym)

#endif
