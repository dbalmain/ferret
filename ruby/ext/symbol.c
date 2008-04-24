#include <stdlib.h>
#include "symbol.h"


FrtSymbol frt_intern_and_free(char *str)
{
    FrtSymbol sym = (FrtSymbol)rb_intern(str);
    free(str);
    return sym;
}
