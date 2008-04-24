#include <string.h>
#include "search.h"
#include "index.h"
#include "field_index.h"
#include "symbol.h"
#include "internal.h"

/***************************************************************************
 *
 * SortField
 *
 ***************************************************************************/

static INLINE SortField *sort_field_alloc(Symbol field,
    SortType type,
    bool reverse,
    int (*compare)(void *index_ptr, Hit *hit1, Hit *hit2),
    void (*get_val)(void *index_ptr, Hit *hit1, Comparable *comparable),
    const FieldIndexClass *field_index_class)
{
    SortField *self         = ALLOC(SortField);
    self->field             = field;
    self->type              = type;
    self->reverse           = reverse;
    self->field_index_class = field_index_class;
    self->compare           = compare;
    self->get_val           = get_val;
    return self;
}

SortField *sort_field_new(Symbol field, SortType type, bool reverse)
{
    SortField *sf = NULL;
    switch (type) {
        case SORT_TYPE_SCORE:
            sf = sort_field_score_new(reverse);
            break;
        case SORT_TYPE_DOC:
            sf = sort_field_doc_new(reverse);
            break;
        case SORT_TYPE_BYTE:
            sf = sort_field_byte_new(field, reverse);
            break;
        case SORT_TYPE_INTEGER:
            sf = sort_field_int_new(field, reverse);
            break;
        case SORT_TYPE_FLOAT:
            sf = sort_field_float_new(field, reverse);
            break;
        case SORT_TYPE_STRING:
            sf = sort_field_string_new(field, reverse);
            break;
        case SORT_TYPE_AUTO:
            sf = sort_field_auto_new(field, reverse);
            break;
    }
    return sf;
}

void sort_field_destroy(void *p)
{
    free(p);
}

/*
 * field:<type>!
 */
char *sort_field_to_s(SortField *self)
{
    char *str;
    char *type = NULL;
    switch (self->type) {
        case SORT_TYPE_SCORE:
            type = "<SCORE>";
            break;
        case SORT_TYPE_DOC:
            type = "<DOC>";
            break;
        case SORT_TYPE_BYTE:
            type = "<byte>";
            break;
        case SORT_TYPE_INTEGER:
            type = "<integer>";
            break;
        case SORT_TYPE_FLOAT:
            type = "<float>";
            break;
        case SORT_TYPE_STRING:
            type = "<string>";
            break;
        case SORT_TYPE_AUTO:
            type = "<auto>";
            break;
    }
    if (self->field) {
        str = ALLOC_N(char, 3 + sym_len(self->field) + strlen(type));
        sprintf(str, "%s:%s%s", S(self->field), type, (self->reverse ? "!" : ""));
    }
    else {
        str = ALLOC_N(char, 2 + strlen(type));
        sprintf(str, "%s%s", type, (self->reverse ? "!" : ""));
    }
    return str;
}

/***************************************************************************
 * ScoreSortField
 ***************************************************************************/

static void sf_score_get_val(void *index, Hit *hit, Comparable *comparable)
{
    (void)index;
    comparable->val.f = hit->score;
}

static int sf_score_compare(void *index_ptr, Hit *hit2, Hit *hit1)
{
    float val1 = hit1->score;
    float val2 = hit2->score;
    (void)index_ptr;

    if (val1 > val2) return 1;
    else if (val1 < val2) return -1;
    else return 0;
}

SortField *sort_field_score_new(bool reverse)
{
    return sort_field_alloc(NULL, SORT_TYPE_SCORE, reverse,
                            &sf_score_compare, &sf_score_get_val, NULL);
}

const SortField SORT_FIELD_SCORE = {
    NULL,               /* field_index_class */
    NULL,               /* field */
    SORT_TYPE_SCORE,    /* type */
    false,              /* reverse */
    &sf_score_compare,  /* compare */
    &sf_score_get_val,  /* get_val */
};

const SortField SORT_FIELD_SCORE_REV = {
    NULL,               /* field_index_class */
    NULL,               /* field */
    SORT_TYPE_SCORE,    /* type */
    true,               /* reverse */
    &sf_score_compare,  /* compare */
    &sf_score_get_val,  /* get_val */
};

/**************************************************************************
 * DocSortField
 ***************************************************************************/

static void sf_doc_get_val(void *index, Hit *hit, Comparable *comparable)
{
    (void)index;
    comparable->val.l = hit->doc;
}

static int sf_doc_compare(void *index_ptr, Hit *hit1, Hit *hit2)
{
    int val1 = hit1->doc;
    int val2 = hit2->doc;
    (void)index_ptr;

    if (val1 > val2) return 1;
    else if (val1 < val2) return -1;
    else return 0;
}

SortField *sort_field_doc_new(bool reverse)
{
    return sort_field_alloc(NULL, SORT_TYPE_DOC, reverse,
                            &sf_doc_compare, &sf_doc_get_val, NULL);
}

const SortField SORT_FIELD_DOC = {
    NULL,               /* field_index_class */
    NULL,               /* field */
    SORT_TYPE_DOC,      /* type */
    false,              /* reverse */
    &sf_doc_compare,    /* compare */
    &sf_doc_get_val,    /* get_val */
};

const SortField SORT_FIELD_DOC_REV = {
    NULL,               /* field_index_class */
    NULL,               /* field */
    SORT_TYPE_DOC,      /* type */
    true,               /* reverse */
    &sf_doc_compare,    /* compare */
    &sf_doc_get_val,    /* get_val */
};

/***************************************************************************
 * ByteSortField
 ***************************************************************************/

static void sf_byte_get_val(void *index, Hit *hit, Comparable *comparable)
{
    comparable->val.l = ((long *)index)[hit->doc];
}

static int sf_byte_compare(void *index, Hit *hit1, Hit *hit2)
{
    long val1 = ((long *)index)[hit1->doc];
    long val2 = ((long *)index)[hit2->doc];
    if (val1 > val2) return 1;
    else if (val1 < val2) return -1;
    else return 0;
}

SortField *sort_field_byte_new(Symbol field, bool reverse)
{
    return sort_field_alloc(field, SORT_TYPE_BYTE, reverse,
                            &sf_byte_compare, &sf_byte_get_val,
                            &BYTE_FIELD_INDEX_CLASS);
}

/***************************************************************************
 * IntegerSortField
 ***************************************************************************/

static void sf_int_get_val(void *index, Hit *hit, Comparable *comparable)
{
    comparable->val.l = ((long *)index)[hit->doc];
}

static int sf_int_compare(void *index, Hit *hit1, Hit *hit2)
{
    long val1 = ((long *)index)[hit1->doc];
    long val2 = ((long *)index)[hit2->doc];
    if (val1 > val2) return 1;
    else if (val1 < val2) return -1;
    else return 0;
}

SortField *sort_field_int_new(Symbol field, bool reverse)
{
    return sort_field_alloc(field, SORT_TYPE_INTEGER, reverse,
                            &sf_int_compare, &sf_int_get_val,
                            &INTEGER_FIELD_INDEX_CLASS);
}

/***************************************************************************
 * FloatSortField
 ***************************************************************************/

static void sf_float_get_val(void *index, Hit *hit, Comparable *comparable)
{
    comparable->val.f = ((float *)index)[hit->doc];
}

static int sf_float_compare(void *index, Hit *hit1, Hit *hit2)
{
    float val1 = ((float *)index)[hit1->doc];
    float val2 = ((float *)index)[hit2->doc];
    if (val1 > val2) return 1;
    else if (val1 < val2) return -1;
    else return 0;
}

SortField *sort_field_float_new(Symbol field, bool reverse)
{
    return sort_field_alloc(field, SORT_TYPE_FLOAT, reverse,
                            &sf_float_compare, &sf_float_get_val,
                            &FLOAT_FIELD_INDEX_CLASS);
}

/***************************************************************************
 * StringSortField
 ***************************************************************************/

static void sf_string_get_val(void *index, Hit *hit, Comparable *comparable)
{
    comparable->val.s
        = ((StringIndex *)index)->values[
        ((StringIndex *)index)->index[hit->doc]];
}

static int sf_string_compare(void *index, Hit *hit1, Hit *hit2)
{
    char *s1 = ((StringIndex *)index)->values[
        ((StringIndex *)index)->index[hit1->doc]];
    char *s2 = ((StringIndex *)index)->values[
        ((StringIndex *)index)->index[hit2->doc]];

    if (s1 == NULL) return s2 ? 1 : 0;
    if (s2 == NULL) return -1;

#ifdef POSH_OS_WIN32
    return strcmp(s1, s2);
#else
    return strcoll(s1, s2);
#endif

    /*
     * TODO: investigate whether it would be a good idea to presort strings.
     *
    int val1 = index->index[hit1->doc];
    int val2 = index->index[hit2->doc];
    if (val1 > val2) return 1;
    else if (val1 < val2) return -1;
    else return 0;
    */
}

SortField *sort_field_string_new(Symbol field, bool reverse)
{
    return sort_field_alloc(field, SORT_TYPE_STRING, reverse,
                            &sf_string_compare, &sf_string_get_val,
                            &STRING_FIELD_INDEX_CLASS);
}

/***************************************************************************
 * AutoSortField
 ***************************************************************************/

SortField *sort_field_auto_new(Symbol field, bool reverse)
{
    return sort_field_alloc(field, SORT_TYPE_AUTO, reverse, NULL, NULL, NULL);
}

/***************************************************************************
 *
 * FieldSortedHitQueue
 *
 ***************************************************************************/

/***************************************************************************
 * Comparator
 ***************************************************************************/

typedef struct Comparator {
    void *index;
    bool  reverse : 1;
    int   (*compare)(void *index_ptr, Hit *hit1, Hit *hit2);
} Comparator;

static Comparator *comparator_new(void *index, bool reverse,
                  int (*compare)(void *index_ptr, Hit *hit1, Hit *hit2))
{
    Comparator *self = ALLOC(Comparator);
    self->index = index;
    self->reverse = reverse;
    self->compare = compare;
    return self;
}

/***************************************************************************
 * Sorter
 ***************************************************************************/

typedef struct Sorter {
    Comparator **comparators;
    int c_cnt;
    Sort *sort;
} Sorter;

#define SET_AUTO(upper_type, lower_type) \
    sf->type = SORT_TYPE_ ## upper_type;\
    sf->field_index_class = &upper_type ## _FIELD_INDEX_CLASS;\
    sf->compare = sf_ ## lower_type ## _compare;\
    sf->get_val = sf_ ## lower_type ## _get_val

static void sort_field_auto_evaluate(SortField *sf, char *text)
{
    int int_val;
    float float_val;
    int text_len = 0, scan_len = 0;

    text_len = (int)strlen(text);
    sscanf(text, "%d%n", &int_val, &scan_len);
    if (scan_len == text_len) {
        SET_AUTO(INTEGER, int);
    } else {
        sscanf(text, "%f%n", &float_val, &scan_len);
        if (scan_len == text_len) {
            SET_AUTO(FLOAT, float);
        } else {
            SET_AUTO(STRING, string);
        }
    }
}
/*
static INLINE void set_auto(SortField *sf,
    SortType type,
    const FieldIndexClass *field_index_class,
    int  (*compare)(void *index_ptr, Hit *hit1, Hit *hit2),
    void (*get_val)(void *index_ptr, Hit *hit1, Comparable *comparable))
{
    sf->type = type;
    sf->field_index_class = field_index_class;
    sf->compare = compare;
    sf->get_val = get_val;
}

static void sort_field_auto_evaluate(SortField *sf, char *text)
{
    int int_val;
    float float_val;
    int text_len = 0, scan_len = 0;

    text_len = (int)strlen(text);
    sscanf(text, "%d%n", &int_val, &scan_len);
    if (scan_len == text_len) {
        set_auto(sf, SORT_TYPE_INTEGER, &INTEGER_FIELD_INDEX_CLASS,
                 sf_int_compare, sf_int_get_val);
    } else {
        sscanf(text, "%f%n", &float_val, &scan_len);
        if (scan_len == text_len) {
            set_auto(sf, SORT_TYPE_FLOAT, &FLOAT_FIELD_INDEX_CLASS,
                     sf_float_compare, sf_float_get_val);
        } else {
            set_auto(sf, SORT_TYPE_STRING, &STRING_FIELD_INDEX_CLASS,
                     sf_string_compare, sf_string_get_val);
        }
    }
}
*/

static Comparator *sorter_get_comparator(SortField *sf, IndexReader *ir)
{
    void *index = NULL;

    if (sf->type > SORT_TYPE_DOC) {
        FieldIndex *field_index = NULL;
        if (sf->type == SORT_TYPE_AUTO) {
            TermEnum *te = ir_terms(ir, sf->field);
            if (!te->next(te) && (ir->num_docs(ir) > 0)) {
                RAISE(ARG_ERROR,
                      "Cannot sort by field \"%s\" as there are no terms "
                      "in that field in the index.", S(sf->field));
            }
            sort_field_auto_evaluate(sf, te->curr_term);
            te->close(te);
        }
        mutex_lock(&ir->field_index_mutex);
        field_index = field_index_get(ir, sf->field, sf->field_index_class);
        mutex_unlock(&ir->field_index_mutex);
        index = field_index->index;
    }
    return comparator_new(index, sf->reverse, sf->compare);
}

static void sorter_destroy(Sorter *self)
{
    int i;

    for (i = 0; i < self->c_cnt; i++) {
        free(self->comparators[i]);
    }
    free(self->comparators);
    free(self);
}

static Sorter *sorter_new(Sort *sort)
{
    Sorter *self = ALLOC(Sorter);
    self->c_cnt = sort->size;
    self->comparators = ALLOC_AND_ZERO_N(Comparator *, self->c_cnt);
    self->sort = sort;
    return self;
}

/***************************************************************************
 * FieldSortedHitQueue
 ***************************************************************************/

static bool fshq_less_than(const void *hit1, const void *hit2)
{
    int cmp = 0;
    printf("Whoops, shouldn't call this.\n");
    if (cmp != 0) {
        return cmp;
    } else {
        return ((Hit *)hit1)->score < ((Hit *)hit2)->score;
    }
}

static INLINE bool fshq_lt(Sorter *sorter, Hit *hit1, Hit *hit2)
{
    Comparator *comp;
    int diff = 0, i;
    for (i = 0; i < sorter->c_cnt && diff == 0; i++) {
        comp = sorter->comparators[i];
        if (comp->reverse) {
            diff = comp->compare(comp->index, hit2, hit1);
        } else {
            diff = comp->compare(comp->index, hit1, hit2);
        }
    }

    if (diff != 0) {
        return diff > 0;
    } else {
        return hit1->doc > hit2->doc;
    }
}

void fshq_pq_down(PriorityQueue *pq)
{
    register int i = 1;
    register int j = 2;     /* i << 1; */
    register int k = 3;     /* j + 1; */
    Hit **heap = (Hit **)pq->heap;
    Hit *node = heap[i];    /* save top node */
    Sorter *sorter = (Sorter *)heap[0];

    if ((k <= pq->size) && fshq_lt(sorter, heap[k], heap[j])) {
        j = k;
    }

    while ((j <= pq->size) && fshq_lt(sorter, heap[j], node)) {
        heap[i] = heap[j];  /* shift up child */
        i = j;
        j = i << 1;
        k = j + 1;
        if ((k <= pq->size) && fshq_lt(sorter, heap[k], heap[j])) {
            j = k;
        }
    }
    heap[i] = node;
}

Hit *fshq_pq_pop(PriorityQueue *pq)
{
    if (pq->size > 0) {
        Hit *hit = (Hit *)pq->heap[1];   /* save first value */
        pq->heap[1] = pq->heap[pq->size];   /* move last to first */
        pq->heap[pq->size] = NULL;
        pq->size--;
        fshq_pq_down(pq);                   /* adjust heap */
        return hit;
    } else {
        return NULL;
    }
}

static INLINE void fshq_pq_up(PriorityQueue *pq)
{
    Hit **heap = (Hit **)pq->heap;
    Hit *node;
    int i = pq->size;
    int j = i >> 1;
    Sorter *sorter = (Sorter *)heap[0];
    node = heap[i];

    while ((j > 0) && fshq_lt(sorter, node, heap[j])) {
        heap[i] = heap[j];
        i = j;
        j = j >> 1;
    }
    heap[i] = node;
}

void fshq_pq_insert(PriorityQueue *pq, Hit *hit)
{
    if (pq->size < pq->capa) {
        Hit *new_hit = ALLOC(Hit);
        memcpy(new_hit, hit, sizeof(Hit));
        pq->size++;
        if (pq->size >= pq->mem_capa) {
            pq->mem_capa <<= 1;
            REALLOC_N(pq->heap, void *, pq->mem_capa);
        }
        pq->heap[pq->size] = new_hit;
        fshq_pq_up(pq);
    } else if (pq->size > 0
               && fshq_lt((Sorter *)pq->heap[0], (Hit *)pq->heap[1], hit)) {
        memcpy(pq->heap[1], hit, sizeof(Hit));
        fshq_pq_down(pq);
    }
}

void fshq_pq_destroy(PriorityQueue *self)
{
    sorter_destroy((Sorter *)self->heap[0]);
    pq_destroy(self);
}

PriorityQueue *fshq_pq_new(int size, Sort *sort, IndexReader *ir)
{
    PriorityQueue *self = pq_new(size, &fshq_less_than, &free);
    int i;
    Sorter *sorter = sorter_new(sort);
    SortField *sf;

    for (i = 0; i < sort->size; i++) {
        sf = sort->sort_fields[i];
        sorter->comparators[i] = sorter_get_comparator(sf, ir);
    }
    self->heap[0] = sorter;

    return self;
}

Hit *fshq_pq_pop_fd(PriorityQueue *pq)
{
    if (pq->size <= 0) {
        return NULL;
    }
    else {
        int j;
        Sorter *sorter = (Sorter *)pq->heap[0];
        const int cmp_cnt = sorter->c_cnt;
        SortField **sort_fields = sorter->sort->sort_fields;
        Hit *hit = (Hit *)pq->heap[1];   /* save first value */
        FieldDoc *field_doc;
        Comparable *comparables;
        Comparator **comparators = sorter->comparators;
        pq->heap[1] = pq->heap[pq->size];   /* move last to first */
        pq->heap[pq->size] = NULL;
        pq->size--;
        fshq_pq_down(pq);                   /* adjust heap */

        field_doc = (FieldDoc *)emalloc(sizeof(FieldDoc)
                                        + sizeof(Comparable) * cmp_cnt);
        comparables = field_doc->comparables;
        memcpy(field_doc, hit, sizeof(Hit));
        field_doc->size = cmp_cnt;

        for (j = 0; j < cmp_cnt; j++) {
            SortField *sf = sort_fields[j];
            Comparator *comparator = comparators[j];
            sf->get_val(comparator->index, hit, &(comparables[j]));
            comparables[j].type = sf->type;
            comparables[j].reverse = comparator->reverse;
        }
        free(hit);
        return (Hit *)field_doc;
    }
}

/***************************************************************************
 * FieldDoc
 ***************************************************************************/

void fd_destroy(FieldDoc *fd)
{
    free(fd);
}

/***************************************************************************
 * FieldDocSortedHitQueue
 ***************************************************************************/

bool fdshq_lt(FieldDoc *fd1, FieldDoc *fd2)
{
    int c = 0, i;
    Comparable *cmps1 = fd1->comparables;
    Comparable *cmps2 = fd2->comparables;

    for (i = 0; i < fd1->size && c == 0; i++) {
        int type = cmps1[i].type;
        switch (type) {
            case SORT_TYPE_SCORE:
                if (cmps1[i].val.f < cmps2[i].val.f) c =  1;
                if (cmps1[i].val.f > cmps2[i].val.f) c = -1;
                break;
            case SORT_TYPE_FLOAT:
                if (cmps1[i].val.f > cmps2[i].val.f) c =  1;
                if (cmps1[i].val.f < cmps2[i].val.f) c = -1;
                break;
            case SORT_TYPE_DOC:
                if (fd1->hit.doc > fd2->hit.doc) c =  1;
                if (fd1->hit.doc < fd2->hit.doc) c = -1;
                break;
            case SORT_TYPE_INTEGER:
                if (cmps1[i].val.l > cmps2[i].val.l) c =  1;
                if (cmps1[i].val.l < cmps2[i].val.l) c = -1;
                break;
            case SORT_TYPE_BYTE:
                if (cmps1[i].val.l > cmps2[i].val.l) c =  1;
                if (cmps1[i].val.l < cmps2[i].val.l) c = -1;
                break;
            case SORT_TYPE_STRING:
                do {
                    char *s1 = cmps1[i].val.s;
                    char *s2 = cmps2[i].val.s;
                    if (s1 == NULL) c = s2 ? 1 : 0;
                    else if (s2 == NULL) c = -1;
#ifdef POSH_OS_WIN32
                    else c = strcmp(s1, s2);
#else
                    else c = strcoll(s1, s2);
#endif
                } while (0);
                break;
            default:
                RAISE(ARG_ERROR, "Unknown sort type: %d.", type);
                break;
        }
        if (cmps1[i].reverse) {
            c = -c;
        }
    }
    if (c == 0) {
        return fd1->hit.doc > fd2->hit.doc;
    }
    else {
        return c > 0;
    }
}

/***************************************************************************
 *
 * Sort
 *
 ***************************************************************************/

#define SORT_INIT_SIZE 4

Sort *sort_new()
{
    Sort *self = ALLOC(Sort);
    self->size = 0;
    self->capa = SORT_INIT_SIZE;
    self->sort_fields = ALLOC_N(SortField *, SORT_INIT_SIZE);
    self->destroy_all = true;
    self->start = 0;

    return self;
}

void sort_clear(Sort *self)
{
    int i;
    if (self->destroy_all) {
        for (i = 0; i < self->size; i++) {
            sort_field_destroy(self->sort_fields[i]);
        }
    }
    self->size = 0;
}

void sort_destroy(void *p)
{
    Sort *self = (Sort *)p;
    sort_clear(self);
    free(self->sort_fields);
    free(self);
}

void sort_add_sort_field(Sort *self, SortField *sf)
{
    if (self->size == self->capa) {
        self->capa <<= 1;
        REALLOC_N(self->sort_fields, SortField *, self->capa);
    }

    self->sort_fields[self->size] = sf;
    self->size++;
}

char *sort_to_s(Sort *self)
{
    int i, len = 20;
    char *s;
    char *str;
    char **sf_strs = ALLOC_N(char *, self->size);

    for (i = 0; i < self->size; i++) {
        sf_strs[i] = s = sort_field_to_s(self->sort_fields[i]);
        len += (int)strlen(s) + 2;
    }

    str = ALLOC_N(char, len);
    s = "Sort[";
    len = (int)strlen(s);
    memcpy(str, s, len);

    s = str + len;
    for (i = 0; i < self->size; i++) {
        s += sprintf(s, "%s, ", sf_strs[i]);
        free(sf_strs[i]);
    }
    free(sf_strs);

    if (self->size > 0) {
        s -= 2;
    }
    sprintf(s, "]");
    return str;
}
