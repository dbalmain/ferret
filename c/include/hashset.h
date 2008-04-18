#ifndef FRT_HASHSET_H
#define FRT_HASHSET_H

#include "hash.h"
#include "global.h"

#define FRT_HS_MIN_SIZE 4
typedef struct FerretHashSetEntry {
    void *elem;
    struct FerretHashSetEntry *next;
    struct FerretHashSetEntry *prev;
} FerretHashSetEntry;

typedef struct FerretHashSet
{
    /* the number of elements in the instance */
    int size;

    /* the first element in the list of elements in the FerretHashSet. The elements
     * will be listed in the order they were added and can be iterated over by
     * following the ->next pointer */
    FerretHashSetEntry *first;

    /* the last element in the list of elements in the FerretHashSet. This is used
     * internally to add elements to the list. */
    FerretHashSetEntry *last;

    /* HashTable used internally */
    FerretHashTable *ht;

    /* Internal: Frees elements added to the FerretHashSet. Should never be NULL */
    frt_free_ft free_elem_i;
} FerretHashSet;

/**
 * Create a new FerretHashSet. The function will allocate a FerretHashSet Struct setting
 * the functions used to hash the objects it will contain and the eq function.
 * This should be used for non-string types.
 *
 * @param hash function to hash objects added to the FerretHashSet
 * @param eq function to determine whether two items are equal
 * @param free_elem function used to free elements as added to the FerretHashSet
 *   when the FerretHashSet if destroyed or duplicate elements are added to the Set
 * @return a newly allocated FerretHashSet structure
 */
extern FerretHashSet *frt_hs_new(frt_hash_ft hash_func,
                                 frt_eq_ft eq_func,
                                 frt_free_ft free_func);

/**
 * Create a new FerretHashSet specifically for strings. This will create a FerretHashSet
 * as if you used frt_hs_new with the standard string hash and eq functions.
 *
 * @param free_elem function used to free elements as added to the FerretHashSet
 *   when the FerretHashSet if destroyed or duplicate elements are added to the Set
 * @return a newly allocated FerretHashSet structure
 */
extern FerretHashSet *frt_hs_new_str(frt_free_ft free_func);

/**
 * Free the memory allocated by the FerretHashSet, but don't free the elements added
 * to the FerretHashSet. If you'd like to free everything in the FerretHashSet you should
 * use frt_hs_destroy
 *
 * @param hs the FerretHashSet to free
 */
extern void frt_hs_free(FerretHashSet *self);

/**
 * Destroy the FerretHashSet including all elements added to the FerretHashSet. If you'd
 * like to free the memory allocated to the FerretHashSet without touching the
 * elements in the FerretHashSet then use frt_hs_free
 *
 * @param hs the FerretHashSet to destroy
 */
extern void frt_hs_destroy(FerretHashSet *self);

/**
 * WARNING: this function may destroy some elements if you add them to a
 * FerretHashSet were equivalent elements already exist, depending on how free_elem
 * was set.
 *
 * Add the element to the FerretHashSet whether or not it was already in the
 * FerretHashSet.
 *
 * When a element is added to the HashTable where it already exists, free_elem
 * is called on it, ie the element you tried to add might get destroyed.
 *
 * @param hs the FerretHashSet to add the element to
 * @param elem the element to add to the FerretHashSet
 * @return one of three values;
 *   <pre>
 *     HASH_KEY_DOES_NOT_EXIST  the element was not already in the FerretHashSet.
 *                              This value is equal to 0 or false
 *     HASH_KEY_SAME            the element was identical (same memory
 *                              pointer) to an existing element so no freeing
 *                              was done
 *     HASH_KEY_EQUAL           the element was equal to an element already in
 *                              the FerretHashSet so the new_elem was freed if
 *                              free_elem was set
 *   </pre>
 */
extern FerretHashKeyStatus frt_hs_add(FerretHashSet *self, void *elem);

/**
 * Add element to the FerretHashSet. If the element already existed in the FerretHashSet
 * and the new element was equal but not the same (same pointer/memory) then
 * don't add the element and return false, otherwise return true.
 *
 * @param hs the FerretHashSet to add the element to
 * @param elem the element to add to the FerretHashSet
 * @return true if the element was successfully added or false otherwise
 */
extern int frt_hs_add_safe(FerretHashSet *self, void *elem);

/**
 * Delete the element from the FerretHashSet. Returns true if the item was
 * successfully deleted or false if the element never existed.
 *
 * @param hs the FerretHashSet to delete from
 * @param elem the element to delete
 * @return true if the element was deleted or false if the element never
 *   existed
 */
extern int frt_hs_del(FerretHashSet *self, void *elem);

/**
 * Remove an item from the FerretHashSet without actually freeing the item. This
 * function should return the item itself so that it can be freed later if
 * necessary.
 *
 * @param hs the FerretHashSet to remove the element from.
 * @param elem the element to remove
 * @param the element that was removed or NULL otherwise
 */
extern void *frt_hs_rem(FerretHashSet *self, void *elem);

/**
 * Check if the element exists and return the appropriate value described
 * bellow.
 *
 * @param hs the FerretHashSet to check in
 * @param elem the element to check for
 * @return one of the following values
 * <pre>
 *     HASH_KEY_DOES_NOT_EXIST  the element was not already in the FerretHashSet.
 *                              This value is equal to 0 or false
 *     HASH_KEY_SAME            the element was identical (same memory
 *                              pointer) to an existing element so no freeing
 *                              was done
 *     HASH_KEY_EQUAL           the element was equal to an element already in
 *                              the FerretHashSet so the new_elem was freed if
 *                              free_elem was set
 *   </pre>
 */
extern FerretHashKeyStatus frt_hs_exists(FerretHashSet *self, void *elem);

/**
 * Merge two HashSets. When a merge is done the merger (self) HashTable is
 * returned and the mergee is destroyed. All elements from mergee that were
 * not found in merger (self) will be added to self, otherwise they will be
 * destroyed.
 *
 * @param self the FerretHashSet to merge into
 * @param other HastSet to be merged into self
 * @return the merged FerretHashSet
 */
extern FerretHashSet *frt_hs_merge(FerretHashSet *self, FerretHashSet *other);

/**
 * Return the original version of +elem+. So if you allocate two elements
 * which are equal and add the first to the FerretHashSet, calling this function
 * with the second element will return the first element from the FerretHashSet.
 */
extern void *frt_hs_orig(FerretHashSet *self, void *elem);

/**
 * Clear all elements from the FerretHashSet. If free_elem was set then use it to
 * free all elements as they are cleared. After the method is called, the
 * HashSets size will be 0.
 *
 * @param self the FerretHashSet to clear
 */
extern void frt_hs_clear(FerretHashSet *self);

/* TODO: finish implementing these functions FerretHashSet
int hs_osf(FerretHashSet *hs, void *elem);
FerretHashSet hs_or(FerretHashSet *hs1, FerretHashSet *h2);
FerretHashSet hs_excl_or(FerretHashSet *hs1, FerretHashSet *h2);
FerretHashSet hs_and(FerretHashSet *hs1, FerretHashSet *h2);
FerretHashSet hs_mask(FerretHashSet *hs1, FerretHashSet *h2);
*/

#endif
