#ifndef FRT_HASHSET_H
#define FRT_HASHSET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hash.h"
#include "global.h"

#define FRT_HS_MIN_SIZE 4
typedef struct FrtHashSetEntry {
    void *elem;
    struct FrtHashSetEntry *next;
    struct FrtHashSetEntry *prev;
} FrtHashSetEntry;

typedef struct FrtHashSet
{
    /* the number of elements in the instance */
    int size;

    /* the first element in the list of elements in the FrtHashSet. The elements
     * will be listed in the order they were added and can be iterated over by
     * following the ->next pointer */
    FrtHashSetEntry *first;

    /* the last element in the list of elements in the FrtHashSet. This is used
     * internally to add elements to the list. */
    FrtHashSetEntry *last;

    /* Hash used internally */
    FrtHash *ht;

    /* Internal: Frees elements added to the FrtHashSet. Should never be NULL */
    frt_free_ft free_elem_i;
} FrtHashSet;

/**
 * Create a new FrtHashSet. The function will allocate a FrtHashSet Struct
 * setting the functions used to hash the objects it will contain and the eq
 * function. This should be used for non-string types.
 *
 * @param hash function to hash objects added to the FrtHashSet
 * @param eq function to determine whether two items are equal
 * @param free_elem function used to free elements as added to the FrtHashSet
 *   when the FrtHashSet if destroyed or duplicate elements are added to the Set
 * @return a newly allocated FrtHashSet structure
 */
extern FrtHashSet *frt_hs_new(frt_hash_ft hash_func,
                                 frt_eq_ft eq_func,
                                 frt_free_ft free_func);

/**
 * Create a new FrtHashSet specifically for strings. This will create a
 * FrtHashSet as if you used frt_hs_new with the standard string hash and eq
 * functions.
 *
 * @param free_elem function used to free elements as added to the FrtHashSet
 *   when the FrtHashSet if destroyed or duplicate elements are added to the Set
 * @return a newly allocated FrtHashSet structure
 */
extern FrtHashSet *frt_hs_new_str(frt_free_ft free_func);

/**
 * Create a new FrtHashSet specifically for pointers. Note that the only way
 * two pointers will be considered equal is if they have the same address. So
 * you can add the string "key" twice if it is stored at two different
 * addresses.
 *
 * @param free_elem function used to free elements as added to the FrtHashSet
 *   when the FrtHashSet if destroyed or duplicate elements are added to the Set
 * @return a newly allocated FrtHashSet structure
 */
extern FrtHashSet *frt_hs_new_ptr(frt_free_ft free_func);

/**
 * Free the memory allocated by the FrtHashSet, but don't free the elements added
 * to the FrtHashSet. If you'd like to free everything in the FrtHashSet you should
 * use frt_hs_destroy
 *
 * @param hs the FrtHashSet to free
 */
extern void frt_hs_free(FrtHashSet *self);

/**
 * Destroy the FrtHashSet including all elements added to the FrtHashSet. If you'd
 * like to free the memory allocated to the FrtHashSet without touching the
 * elements in the FrtHashSet then use frt_hs_free
 *
 * @param hs the FrtHashSet to destroy
 */
extern void frt_hs_destroy(FrtHashSet *self);

/**
 * WARNING: this function may destroy some elements if you add them to a
 * FrtHashSet were equivalent elements already exist, depending on how free_elem
 * was set.
 *
 * Add the element to the FrtHashSet whether or not it was already in the
 * FrtHashSet.
 *
 * When a element is added to the Hash where it already exists, free_elem
 * is called on it, ie the element you tried to add might get destroyed.
 *
 * @param hs the FrtHashSet to add the element to
 * @param elem the element to add to the FrtHashSet
 * @return one of three values;
 *   <pre>
 *     HASH_KEY_DOES_NOT_EXIST  the element was not already in the FrtHashSet.
 *                              This value is equal to 0 or false
 *     HASH_KEY_SAME            the element was identical (same memory
 *                              pointer) to an existing element so no freeing
 *                              was done
 *     HASH_KEY_EQUAL           the element was equal to an element already in
 *                              the FrtHashSet so the new_elem was freed if
 *                              free_elem was set
 *   </pre>
 */
extern FrtHashKeyStatus frt_hs_add(FrtHashSet *self, void *elem);

/**
 * Add element to the FrtHashSet. If the element already existed in the FrtHashSet
 * and the new element was equal but not the same (same pointer/memory) then
 * don't add the element and return false, otherwise return true.
 *
 * @param hs the FrtHashSet to add the element to
 * @param elem the element to add to the FrtHashSet
 * @return true if the element was successfully added or false otherwise
 */
extern int frt_hs_add_safe(FrtHashSet *self, void *elem);

/**
 * Delete the element from the FrtHashSet. Returns true if the item was
 * successfully deleted or false if the element never existed.
 *
 * @param hs the FrtHashSet to delete from
 * @param elem the element to delete
 * @return true if the element was deleted or false if the element never
 *   existed
 */
extern int frt_hs_del(FrtHashSet *self, const void *elem);

/**
 * Remove an item from the FrtHashSet without actually freeing the item. This
 * function should return the item itself so that it can be freed later if
 * necessary.
 *
 * @param hs the FrtHashSet to remove the element from.
 * @param elem the element to remove
 * @param the element that was removed or NULL otherwise
 */
extern void *frt_hs_rem(FrtHashSet *self, const void *elem);

/**
 * Check if the element exists and return the appropriate value described
 * bellow.
 *
 * @param hs the FrtHashSet to check in
 * @param elem the element to check for
 * @return one of the following values
 * <pre>
 *     HASH_KEY_DOES_NOT_EXIST  the element was not already in the FrtHashSet.
 *                              This value is equal to 0 or false
 *     HASH_KEY_SAME            the element was identical (same memory
 *                              pointer) to an existing element so no freeing
 *                              was done
 *     HASH_KEY_EQUAL           the element was equal to an element already in
 *                              the FrtHashSet so the new_elem was freed if
 *                              free_elem was set
 *   </pre>
 */
extern FrtHashKeyStatus frt_hs_exists(FrtHashSet *self, const void *elem);

/**
 * Merge two HashSets. When a merge is done the merger (self) Hash is
 * returned and the mergee is destroyed. All elements from mergee that were
 * not found in merger (self) will be added to self, otherwise they will be
 * destroyed.
 *
 * @param self the FrtHashSet to merge into
 * @param other HastSet to be merged into self
 * @return the merged FrtHashSet
 */
extern FrtHashSet *frt_hs_merge(FrtHashSet *self, FrtHashSet *other);

/**
 * Return the original version of +elem+. So if you allocate two elements
 * which are equal and add the first to the FrtHashSet, calling this function
 * with the second element will return the first element from the FrtHashSet.
 */
extern void *frt_hs_orig(FrtHashSet *self, const void *elem);

/**
 * Clear all elements from the FrtHashSet. If free_elem was set then use it to
 * free all elements as they are cleared. After the method is called, the
 * HashSets size will be 0.
 *
 * @param self the FrtHashSet to clear
 */
extern void frt_hs_clear(FrtHashSet *self);

/* TODO: finish implementing these functions FrtHashSet
int hs_osf(FrtHashSet *hs, void *elem);
FrtHashSet hs_or(FrtHashSet *hs1, FrtHashSet *h2);
FrtHashSet hs_excl_or(FrtHashSet *hs1, FrtHashSet *h2);
FrtHashSet hs_and(FrtHashSet *hs1, FrtHashSet *h2);
FrtHashSet hs_mask(FrtHashSet *hs1, FrtHashSet *h2);
*/

#ifdef __cplusplus
} // extern "C"
#endif

#endif
