#include <string.h>
#include "search.h"
#include "index.h"

/***************************************************************************
 *
 * SortField
 *
 ***************************************************************************/

f_u32 sort_field_hash(const void *p)
{
    SortField *self = (SortField *)p;
    return str_hash(self->field) ^ (self->type*37);
}

int sort_field_eq(const void *p1, const void *p2)
{
    SortField *key1 = (SortField *)p1;
    SortField *key2 = (SortField *)p2;
    return (strcmp(key1->field, key2->field) == 0)
        && key1->type == key2->type;
}

static int sort_field_cache_eq(const void *p1, const void *p2)
{
    SortField *key1 = (SortField *)p1;
    SortField *key2 = (SortField *)p2;
    int equal = (strcmp(key1->field, key2->field) == 0)
        && key1->type == key2->type;
    /* 
     * TODO: This could probable be done more cleanly. It is basically a hack.
     * If the sort field is an auto field then it was evaluated before it was
     * entered into the cache so we need to pass the compare function back to
     * the new sort field.
     */
    if (equal && (key1->type == SORT_TYPE_AUTO)) {
        key2->compare = key1->compare;
    }
    return equal;
}

static SortField *sort_field_clone(SortField *self)
{
    SortField *clone = ALLOC(SortField);
    memcpy(clone, self, sizeof(SortField));
    mutex_init(&clone->mutex, NULL);
    clone->field = estrdup(self->field);
    return clone;
}

static SortField *sort_field_alloc(char *field, int type, bool reverse)
{
    SortField *self = ALLOC(SortField);
    mutex_init(&self->mutex, NULL);
    self->field         = field ? estrdup(field) : NULL;
    self->type          = type;
    self->reverse       = reverse;
    self->index         = NULL;
    self->destroy_index = &free;
    self->compare       = NULL;
    return self;
}

SortField *sort_field_new(char *field, int type, bool reverse)
{
    SortField *sf = NULL;
    switch (type) {
        case SORT_TYPE_SCORE:
            sf = sort_field_score_new(reverse);
            break;
        case SORT_TYPE_DOC:
            sf = sort_field_doc_new(reverse);
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
    SortField *self = (SortField *)p;
    if (self->index) {
        self->destroy_index(self->index);
    }
    free(self->field);
    mutex_destroy(&self->mutex);
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
        str = ALLOC_N(char, 10 + strlen(self->field) + strlen(type));
        sprintf(str, "%s:%s%s", self->field, type, (self->reverse ? "!" : ""));
    } else {
        str = ALLOC_N(char, 10 + strlen(type));
        sprintf(str, "%s%s", type, (self->reverse ? "!" : ""));
    }
    return str;
}

/***************************************************************************
 * ScoreSortField
 ***************************************************************************/

int sf_score_compare(void *index_ptr, Hit *hit2, Hit *hit1)
{
    (void)index_ptr;
    float val1 = hit1->score;
    float val2 = hit2->score;
    if (val1 > val2) return 1;
    else if (val1 < val2) return -1;
    else return 0;
}

SortField *sort_field_score_new(bool reverse)
{
    SortField *self = sort_field_alloc(NULL, SORT_TYPE_SCORE, reverse);
    self->compare = &sf_score_compare;
    return self;
}

const SortField SORT_FIELD_SCORE = {
    MUTEX_INITIALIZER,
    NULL,               /* field */
    SORT_TYPE_SCORE,    /* type */
    false,              /* reverse */
    NULL,               /* index */
    &sf_score_compare,  /* compare */
    NULL,               /* create_index */
    NULL,               /* destroy_index */
    NULL,               /* handle_term */
};

const SortField SORT_FIELD_SCORE_REV = {
    MUTEX_INITIALIZER,
    NULL,               /* field */
    SORT_TYPE_SCORE,    /* type */
    true,               /* reverse */
    NULL,               /* index */
    &sf_score_compare,  /* compare */
    NULL,               /* create_index */
    NULL,               /* destroy_index */
    NULL,               /* handle_term */
};

/**************************************************************************
 * DocSortField
 ***************************************************************************/

int sf_doc_compare(void *index_ptr, Hit *hit1, Hit *hit2)
{
    (void)index_ptr;
    int val1 = hit1->doc;
    int val2 = hit2->doc;
    if (val1 > val2) return 1;
    else if (val1 < val2) return -1;
    else return 0;
}

SortField *sort_field_doc_new(bool reverse)
{
    SortField *self = sort_field_alloc(NULL, SORT_TYPE_DOC, reverse);
    self->compare = &sf_doc_compare;
    return self;
}

const SortField SORT_FIELD_DOC = {
    MUTEX_INITIALIZER,
    NULL,               /* field */
    SORT_TYPE_DOC,      /* type */
    true,               /* reverse */
    NULL,               /* index */
    &sf_doc_compare,    /* compare */
    NULL,               /* create_index */
    NULL,               /* destroy_index */
    NULL,               /* handle_term */
};

const SortField SORT_FIELD_DOC_REV = {
    MUTEX_INITIALIZER,
    NULL,               /* field */
    SORT_TYPE_DOC,      /* type */
    false,              /* reverse */
    NULL,               /* index */
    &sf_doc_compare,    /* compare */
    NULL,               /* create_index */
    NULL,               /* destroy_index */
    NULL,               /* handle_term */
};

/***************************************************************************
 * IntegerSortField
 ***************************************************************************/

int sf_int_compare(void *index, Hit *hit1, Hit *hit2)
{
    int val1 = ((int *)index)[hit1->doc];
    int val2 = ((int *)index)[hit2->doc];
    if (val1 > val2) return 1;
    else if (val1 < val2) return -1;
    else return 0;
}

void *sf_int_create_index(int size)
{
    return ALLOC_N(int, size);
}

void sf_int_handle_term(void *index_ptr, TermDocEnum *tde, char *text)
{
    int *index = (int *)index_ptr;
    int val;
    sscanf(text, "%d", &val);
    while (tde->next(tde)) {
        index[tde->doc_num(tde)] = val;
    }
}

void sort_field_int_methods(SortField *self)
{
    self->compare = &sf_int_compare;
    self->create_index = &sf_int_create_index;
    self->handle_term = &sf_int_handle_term;
}

SortField *sort_field_int_new(char *field, bool reverse)
{
    SortField *self = sort_field_alloc(field, SORT_TYPE_INTEGER, reverse);
    sort_field_int_methods(self);
    return self;
}

/***************************************************************************
 * FloatSortField
 ***************************************************************************/

int sf_float_compare(void *index, Hit *hit1, Hit *hit2)
{
    float val1 = ((float *)index)[hit1->doc];
    float val2 = ((float *)index)[hit2->doc];
    if (val1 > val2) return 1;
    else if (val1 < val2) return -1;
    else return 0;
}

void *sf_float_create_index(int size)
{
    return ALLOC_N(float, size);
}

void sf_float_handle_term(void *index_ptr, TermDocEnum *tde, char *text)
{
    float *index = (float *)index_ptr;
    float val;
    sscanf(text, "%g", &val);
    while (tde->next(tde)) {
        index[tde->doc_num(tde)] = val;
    }
}

void sort_field_float_methods(SortField *self)
{
    self->compare = &sf_float_compare;
    self->create_index = &sf_float_create_index;
    self->handle_term = &sf_float_handle_term;
}

SortField *sort_field_float_new(char *field, bool reverse)
{
    SortField *self = sort_field_alloc(field, SORT_TYPE_FLOAT, reverse);
    sort_field_float_methods(self);
    return self;
}

/***************************************************************************
 * StringSortField
 ***************************************************************************/

#define VALUES_ARRAY_START_SIZE 8
typedef struct StringIndex {
    int size;
    int *index;
    char **values;
    int v_cnt;
    int v_size;
} StringIndex;

int sf_string_compare(void *index_ptr, Hit *hit1, Hit *hit2)
{
    StringIndex *index = (StringIndex *)index_ptr;
    return strcoll(index->values[index->index[hit1->doc]],
                   index->values[index->index[hit2->doc]]);
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

void *sf_string_create_index(int size)
{
    StringIndex *self = ALLOC_AND_ZERO(StringIndex);
    self->size = size;
    self->index = ALLOC_AND_ZERO_N(int, size);
    self->v_size = VALUES_ARRAY_START_SIZE;
    self->values = ALLOC_N(char *, VALUES_ARRAY_START_SIZE);
    return self;
}

void sf_string_destroy_index(void *p)
{
    StringIndex *self = (StringIndex *)p;
    int i;
    free(self->index);
    for (i = 0; i < self->v_cnt; i++) {
        free(self->values[i]);
    }
    free(self->values);
    free(self);
}

void sf_string_handle_term(void *index_ptr, TermDocEnum *tde, char *text)
{
    StringIndex *index = (StringIndex *)index_ptr;
    if (index->v_cnt >= index->v_size) {
        index->v_size *= 2;
        index->values = REALLOC_N(index->values, char *, index->v_size);
    }
    index->values[index->v_cnt] = estrdup(text);
    while (tde->next(tde)) {
        index->index[tde->doc_num(tde)] = index->v_cnt;
    }
    index->v_cnt++;
}

void sort_field_string_methods(SortField *self)
{
    self->compare = &sf_string_compare;
    self->create_index = &sf_string_create_index;
    self->destroy_index = &sf_string_destroy_index;
    self->handle_term = &sf_string_handle_term;
}

SortField *sort_field_string_new(char *field, bool reverse)
{
    SortField *self = sort_field_alloc(field, SORT_TYPE_STRING, reverse);
    sort_field_string_methods(self);
    return self;
}

/***************************************************************************
 * AutoSortField
 ***************************************************************************/

void sort_field_auto_evaluate(SortField *sf, char *text)
{
    int int_val;
    float float_val;
    int text_len = 0, scan_len = 0;

    text_len = (int)strlen(text);
    sscanf(text, "%d%n", &int_val, &scan_len);
    if (scan_len == text_len) {
        sort_field_int_methods(sf);
    } else {
        sscanf(text, "%f%n", &float_val, &scan_len);
        if (scan_len == text_len) {
            sort_field_float_methods(sf);
        } else {
            sort_field_string_methods(sf);
        }
    }
}

SortField *sort_field_auto_new(char *field, bool reverse)
{
    return sort_field_alloc(field, SORT_TYPE_AUTO, reverse);
}

/***************************************************************************
 *
 * FieldCache
 *
 ***************************************************************************/

void *field_cache_get_index(IndexReader *ir, SortField *sf)
{
    void *index = NULL;
    int length = 0;
    TermEnum *volatile te = NULL;
    TermDocEnum *volatile tde = NULL;
    SortField *sf_clone;
    const int field_num = fis_get_field_num(ir->fis, sf->field);

    if (field_num < 0) {
        RAISE(ARG_ERROR,
              "Cannot sort by field \"%s\". It doesn't exist in the index.",
              sf->field);
    }

    mutex_lock(&sf->mutex);
    if (!ir->sort_cache) {
        ir->sort_cache = h_new(&sort_field_hash, &sort_field_cache_eq,
                               &sort_field_destroy, NULL);
    }
    index = h_get(ir->sort_cache, sf);

    if (index == NULL) {
        length = ir->max_doc(ir);
        if (length > 0) {
            TRY
                tde = ir->term_docs(ir);
                te = ir->terms(ir, field_num);
                if (!te->next(te)) {
                    RAISE(ARG_ERROR,
                          "Cannot sort by field \"%s\" as there are no terms "
                          "in that field in the index.", sf->field);
                }

                if (sf->type == SORT_TYPE_AUTO) {
                    sort_field_auto_evaluate(sf, te->curr_term);
                }

                index = sf->create_index(length);

                do {
                    tde->seek_te(tde, te);
                    sf->handle_term(index, tde, te->curr_term);
                } while (te->next(te));
            XFINALLY
                tde->close(tde);
            te->close(te);
            XENDTRY
        }
        sf_clone = sort_field_clone(sf);
        sf_clone->index = index;
        h_set(ir->sort_cache, sf_clone, index);
    }
    mutex_unlock(&sf->mutex);
    return index;
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

Comparator *comparator_new(void *index, bool reverse,
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
} Sorter;

Comparator *sorter_get_comparator(SortField *sf, IndexReader *ir)
{
    void *index = NULL;

    if (sf->type > SORT_TYPE_DOC) {
        index = field_cache_get_index(ir, sf);
    }
    return comparator_new(index, sf->reverse, sf->compare);
}

void sorter_destroy(void *p)
{
    int i;
    Sorter *self = (Sorter *)p;

    for (i = 0; i < self->c_cnt; i++) {
        free(self->comparators[i]);
    }
    free(self->comparators);
    free(self);
}

Sorter *sorter_new(int size)
{
    Sorter *self = ALLOC(Sorter);
    self->c_cnt = size;
    self->comparators = ALLOC_AND_ZERO_N(Comparator *, size);
    return self;
}

/***************************************************************************
 * FieldSortedHitQueue
 ***************************************************************************/

bool fshq_less_than(const void *hit1, const void *hit2)
{
    int cmp = 0;
    printf("Whoops, shouldn't call this.\n");
    if (cmp != 0) {
        return cmp;
    } else {
        return ((Hit *)hit1)->score < ((Hit *)hit2)->score;
    }
}

inline bool fshq_lt(Hit *sorter_ptr, Hit *hit1, Hit *hit2)
{
    Sorter *sorter = (Sorter *)sorter_ptr;
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
    register int j = 2; //i << 1;
    register int k = 3; //j + 1;
    Hit **heap = (Hit **)pq->heap;
    Hit *node = heap[i]; // save top node

    if ((k <= pq->size) && fshq_lt(heap[0], heap[k], heap[j])) {
        j = k;
    }

    while ((j <= pq->size) && fshq_lt(heap[0], heap[j], node)) {
        heap[i] = heap[j];      // shift up child
        i = j;
        j = i << 1;
        k = j + 1;
        if ((k <= pq->size) && fshq_lt(heap[0], heap[k], heap[j])) {
            j = k;
        }
    }
    heap[i] = node;
}

Hit *fshq_pq_pop(PriorityQueue *pq)
{
    if (pq->size > 0) {
        Hit *result = (Hit *)pq->heap[1];  // save first value
        pq->heap[1] = pq->heap[pq->size]; // move last to first
        pq->heap[pq->size] = NULL;
        pq->size--;
        fshq_pq_down(pq);                   // adjust heap
        return result;
    } else {
        return NULL;
    }
}

inline void fshq_pq_up(PriorityQueue *pq)
{
    Hit **heap = (Hit **)pq->heap;
    Hit *node;
    int i = pq->size;
    int j = i >> 1;
    node = heap[i];

    while ((j > 0) && fshq_lt(heap[0], node, heap[j])) {
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
        pq->heap[pq->size] = new_hit;
        fshq_pq_up(pq);
    } else if (pq->size > 0
               && fshq_lt((Hit *)pq->heap[0], (Hit *)pq->heap[1], hit)) {
        memcpy(pq->heap[1], hit, sizeof(Hit));
        fshq_pq_down(pq);
    }
}

void fshq_pq_destroy(PriorityQueue *self)
{
    sorter_destroy(self->heap[0]);
    pq_destroy(self);
}

PriorityQueue *fshq_pq_new(int size, Sort *sort, IndexReader *ir)
{
    PriorityQueue *self = pq_new(size, &fshq_less_than, &free);
    int i;
    Sorter *sorter = sorter_new(sort->sf_cnt);
    SortField *sf;

    for (i = 0; i < sort->sf_cnt; i++) {
        sf = sort->sort_fields[i];
        sorter->comparators[i] = sorter_get_comparator(sf, ir);
    }
    self->heap[0] = sorter;

    return self;
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
    self->sf_cnt = 0;
    self->sf_capa = SORT_INIT_SIZE;
    self->sort_fields = ALLOC_N(SortField *, SORT_INIT_SIZE);
    self->destroy_all = true;

    return self;
}

void sort_clear(Sort *self)
{
    int i;
    if (self->destroy_all) {
        for (i = 0; i < self->sf_cnt; i++) {
            sort_field_destroy(self->sort_fields[i]);
        }
    }
    self->sf_cnt = 0;
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
    if (self->sf_cnt == self->sf_capa) {
        self->sf_capa *= 2;
        REALLOC_N(self->sort_fields, SortField *, self->sf_capa);
    }

    self->sort_fields[self->sf_cnt] = sf;
    self->sf_cnt++;
}

char *sort_to_s(Sort *self)
{
    int i, len = 20;
    char *s;
    char *str;
    char **sf_strs = ALLOC_N(char *, self->sf_cnt);

    for (i = 0; i < self->sf_cnt; i++) {
        sf_strs[i] = s = sort_field_to_s(self->sort_fields[i]);
        len += (int)strlen(s) + 2;
    }

    str = ALLOC_N(char, len);
    s = "Sort[";
    len = (int)strlen(s);
    memcpy(str, s, len);

    s = str + len;
    for (i = 0; i < self->sf_cnt; i++) {
        sprintf(s, "%s, ", sf_strs[i]);
        s += (int)strlen(s);
        free(sf_strs[i]);
    }
    free(sf_strs);

    if (self->sf_cnt > 0) {
        s -= 2;
    }
    sprintf(s, "]");
    return str;
}
