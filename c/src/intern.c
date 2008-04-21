#include "intern.h"
#include "hash.h"
#include "internal.h"

static Hash *symbol_table = NULL;


void intern_init()
{
    symbol_table = h_new_str(free, NULL);
    register_for_cleanup(symbol_table, (free_ft)&h_destroy);
}

const char *intern(const char *str)
{
    char *symbol = (char *)h_get(symbol_table, str);
    if (!symbol) {
        symbol = estrdup(str);
        h_set(symbol_table, symbol, symbol);
    }
    return symbol;
}

const char *intern_and_free(char *str)
{
    char *symbol = (char *)h_get(symbol_table, str);
    if (!symbol) {
        symbol = str;
        h_set(symbol_table, symbol, symbol);
    }
    else {
        free(str);
    }
    return symbol;
}
