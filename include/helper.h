#ifndef FRT_HELPER_H
#define FRT_HELPER_H

#include "defines.h"

extern inline int hlp_string_diff(register const char *const s1,
                                  register const char *const s2);
extern f_u32 float2int(float f);
extern float int2float(f_u32 i32);
extern float byte2float(unsigned char b);
extern unsigned char float2byte(float f);

#endif
