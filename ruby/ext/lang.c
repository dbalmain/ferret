#include "lang.h"


struct timeval rb_time_interval _((VALUE));
extern void frt_micro_sleep(const int micro_seconds)
{
    rb_thread_wait_for(rb_time_interval(rb_float_new((double)micro_seconds/1000000.0)));
}

