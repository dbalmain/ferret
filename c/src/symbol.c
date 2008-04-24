#include "symbol.h"
#include "hash.h"
#include "internal.h"

static Hash *symbol_table = NULL;


void symbol_init()
{
    symbol_table = h_new_str(free, NULL);
    register_for_cleanup(symbol_table, (free_ft)&h_destroy);
}

Symbol intern(const char *str)
{
    Symbol symbol = (Symbol)h_get(symbol_table, str);
    if (!symbol) {
        symbol = (Symbol)estrdup(str);
        h_set(symbol_table, (void *)symbol, (void *)symbol);
    }
    return symbol;
}

Symbol intern_and_free(char *str)
{
    Symbol symbol = (Symbol)h_get(symbol_table, str);
    if (!symbol) {
        symbol = (Symbol)str;
        h_set(symbol_table, str, str);
    }
    else {
        free(str);
    }
    return symbol;
}
