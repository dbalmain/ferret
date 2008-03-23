#include "hashset.h"
#include <string.h>

/*
 * The HashSet contains an array +elems+ of the elements that have been added.
 * It always has +size+ elements so +size+ ane +elems+ can be used to iterate
 * over all alements in the HashSet. It also uses a HashTable to keep track of
 * which elements have been added and their index in the +elems+ array.
 */
static HashSet *hs_alloc(free_ft free_func)
{
    HashSet *hs = ALLOC(HashSet);
    hs->size = 0;
    hs->capa = HS_MIN_SIZE;
    hs->elems = ALLOC_N(void *, HS_MIN_SIZE);
    hs->free_elem_i = free_func ? free_func : &dummy_free;
    return hs;
}

HashSet *hs_new(hash_ft hash_func, eq_ft eq_func, free_ft free_func)
{
    HashSet *hs = hs_alloc(free_func);
    hs->ht = h_new(hash_func, eq_func, NULL, &free);
    return hs;
}

HashSet *hs_new_str(free_ft free_func)
{
    HashSet *hs = hs_alloc(free_func);
    hs->ht = h_new_str((free_ft) NULL, &free);
    return hs;
}

void hs_free(HashSet *hs)
{
    h_destroy(hs->ht);
    free(hs->elems);
    free(hs);
}

void hs_clear(HashSet *hs)
{
    int i;
    for (i = hs->size - 1; i >= 0; i--) {
        hs_del(hs, hs->elems[i]);
    }
}

void hs_destroy(HashSet *hs)
{
    int i;
    if (hs->free_elem_i != &dummy_free) {
        for (i = 0; i < hs->size; i++) {
            hs->free_elem_i(hs->elems[i]);
        }
    }
    h_destroy(hs->ht);
    free(hs->elems);
    free(hs);
}

int hs_add(HashSet *hs, void *elem)
{
    int has_elem = h_has_key(hs->ht, elem);
    switch (has_elem)
    {
        /* We don't want to keep two of the same elem so free if necessary */
        case HASH_KEY_EQUAL:
            hs->free_elem_i(elem);
            return has_elem;

        /* No need to do anything */
        case HASH_KEY_SAME:
            return has_elem;
    }

    /* add the elem to the array, resizing if necessary */
    if (hs->size >= hs->capa) {
        hs->capa *= 2;
        REALLOC_N(hs->elems, void *, hs->capa);
    }
    hs->elems[hs->size] = elem;
    h_set(hs->ht, elem, imalloc(hs->size));
    hs->size++;
    return has_elem;
}

int hs_add_safe(HashSet *hs, void *elem)
{
    switch(h_has_key(hs->ht, elem))
    {
        /* element can't be added */
        case HASH_KEY_EQUAL: return false;

        /* the exact same element has already been added */
        case HASH_KEY_SAME : return true;
    }

    /* add the elem to the array, resizing if necessary */
    if (hs->size >= hs->capa) {
        hs->capa *= 2;
        REALLOC_N(hs->elems, void *, hs->capa);
    }

    hs->elems[hs->size] = elem;
    h_set(hs->ht, elem, imalloc(hs->size));
    hs->size++;
    return true;
}

int hs_del(HashSet *hs, void *elem)
{
    void *tmp_elem = hs_rem(hs, elem);
    if (tmp_elem != NULL) {
        hs->free_elem_i(tmp_elem);
        return 1;
    }
    return 0;
}

void *hs_rem(HashSet *hs, void *elem)
{
    void *ret_elem;
    int i, *index = (int *)h_get(hs->ht, elem);
    if (index == NULL) {
        return NULL;
    }

    i = *index;
    ret_elem = hs->elems[i];
    h_del(hs->ht, elem);
    hs->size--;
    for (; i < hs->size; ++i) {
        hs->elems[i] = hs->elems[i+1];
        h_set(hs->ht, hs->elems[i], imalloc(i));
    }
    return ret_elem;
}

int hs_exists(HashSet *hs, void *elem)
{
    return h_has_key(hs->ht, elem);
}

HashSet *hs_merge(HashSet *hs, HashSet * other)
{
    int i;
    for (i = 0; i < other->size; i++) {
        hs_add(hs, other->elems[i]);
    }
    /* Now free the other hashset. It is no longer needed. No need, however, to
     * delete the elements as they're either destroyed or in the new hashset */
    hs_free(other);
    return hs;
}

void *hs_orig(HashSet *hs, void *elem)
{
    int *index = h_get(hs->ht, elem);
    return index ? hs->elems[*index] : NULL;
}
