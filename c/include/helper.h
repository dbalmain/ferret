#ifndef FRT_HELPER_H
#define FRT_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"

extern int frt_hlp_string_diff(register const char *const s1,
                               register const char *const s2);
extern frt_i32 frt_float2int(float f);
extern float frt_int2float(frt_i32 i32);
extern float frt_byte2float(unsigned char b);
extern unsigned char frt_float2byte(float f);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
