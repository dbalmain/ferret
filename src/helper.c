#include "helper.h"

inline int hlp_string_diff(register const char *const s1,
                                   register const char *const s2)
{
    register int i = 0;
    while (s1[i] && (s1[i] == s2[i])) {
        i++;
    }
    return i;
}
