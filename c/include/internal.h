#ifndef FRT_INTERNAL_H
#define FRT_INTERNAL_H

#include "priorityqueue.h"

/* Types */
#define PriorityQueue FerretPriorityQueue

/* Functions */
#define lt_ft      frt_lt_ft
#define pq_clear   frt_pq_clear
#define pq_clone   frt_pq_clone
#define pq_destroy frt_pq_destroy
#define pq_down    frt_pq_down
#define pq_free    frt_pq_free
#define pq_full    frt_pq_full
#define pq_insert  frt_pq_insert
#define pq_new     frt_pq_new
#define pq_pop     frt_pq_pop
#define pq_push    frt_pq_push
#define pq_top     frt_pq_top

#endif
