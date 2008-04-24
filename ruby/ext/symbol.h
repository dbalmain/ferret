#ifndef FRT_SYMBOL_H
#define FRT_SYMBOL_H

#include <ruby.h>

typedef void *FrtSymbol;

VALUE rb_id2str(ID id);
#define frt_symbol_init()
#define frt_intern(str) (FrtSymbol)rb_intern(str)
extern FrtSymbol frt_intern_and_free(char *str);

#define FRT_I frt_intern
#define FRT_InF frt_intern_and_free
#define FRT_S(_sym) rb_id2name((ID)_sym)

#define FSYM2SYM(_sym) ID2SYM((ID)_sym)
#define SYM2FSYM(_sym) (FrtSymbol)SYM2ID(_sym)

#define frt_sym_hash(sym) frt_str_hash(rb_id2name((ID)sym))
#define frt_sym_len(sym) strlen(rb_id2name((ID)sym))

#endif
