#include <string.h>
#include "index.h"
#include "array.h"
#include "helper.h"
#include "symbol.h"
#include "internal.h"

/****************************************************************************
 *
 * TermVector
 *
 ****************************************************************************/

void tv_destroy(TermVector *tv)
{
    int i = tv->term_cnt;
    while (i > 0) {
        i--;
        free(tv->terms[i].text);
        free(tv->terms[i].positions);
    }
    free(tv->offsets);
    free(tv->terms);
    free(tv);
}

int tv_get_tv_term_index(TermVector *tv, const char *term)
{
    int lo = 0;                 /* search starts array */
    int hi = tv->term_cnt - 1;  /* for 1st element < n, return its index */
    int mid;
    int cmp;
    char *mid_term;

    while (hi > lo) {
        mid = (lo + hi) >> 1;
        mid_term = tv->terms[mid].text;
        cmp = strcmp(term, mid_term);
        if (cmp < 0) {
            hi = mid - 1;
        }
        else if (cmp > 0) {
            lo = mid + 1;
        }
        else {                  /* found a match */
            return mid;
        }
    }
    if (hi >= 0 && strcmp(term, tv->terms[hi].text) == 0) {
        return hi;
    }
    return -1;
}

extern TVTerm *tv_get_tv_term(TermVector *tv, const char *term)
{
    int index = tv_get_tv_term_index(tv, term);
    if (index >= 0) {
        return &(tv->terms[index]);
    }
    else {
        return NULL;
    }
}
