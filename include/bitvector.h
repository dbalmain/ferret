#ifndef FRT_BIT_VECTOR_H
#define FRT_BIT_VECTOR_H

#include "global.h"

#define BV_INIT_CAPA 256
typedef struct BitVector
{
    /** The bits are held in an array of 32-bit integers */
    f_u32 *bits;

    /** size is equal to 1 + the highest order bit set */
    int size;

    /** capa is the number of words (U32) allocated for the bits */
    int capa;

    /** count is the running count of bits set. This is kept up to date by
     * bv_set and bv_unset. You can reset this value by calling bv_recount */
    int count;

    /** curr_bit is used by scan_next to record the previously  scanned bit */
    int curr_bit;
} BitVector;

/**
 * Create a new BitVector with a capacity of +BV_INIT_CAPA+. Note that the
 * BitVector is growable and will adjust it's capacity when you use bv_set.
 *
 * @return BitVector with a capacity of +BV_INIT_CAPA+.
 */
extern BitVector *bv_create();

/**
 * Create a new BitVector with a capacity of +capa+. Note that the BitVector
 * is growable and will adjust it's capacity when you use bv_set.
 *
 * @param capa the initial capacity of the BitVector
 * @return BitVector with a capacity of +capa+.
 */
extern BitVector *bv_create_capa(int capa);

/**
 * Destroy a BitVector, freeing all memory allocated to that BitVector
 *
 * @param bv BitVector to destroy
 */
extern void bv_destroy(BitVector * bv);

/**
 * Set the bit at position +index+. If +index+ is outside of the range of the
 * BitVector, that is >= BitVector.size, BitVector.size will be set to +index+
 * + 1. If it is greater than the capacity of the BitVector, the capacity will
 * be expanded to accomodate.
 *
 * @param bv the BitVector to set the bit in
 * @param index the index of the bit to set
 */
extern void bv_set(BitVector * bv, int index);

/**
 * Unsafely set the bit at position +index+. If you choose to use this
 * function you must create the BitVector with a large enough capacity to
 * accomodate all of the bv_set_fast operations. You must also set bits in
 * order and only one time per bit. Otherwise, use the safe bv_set function.
 *
 * So this is ok;
 * <pre>
 *   BitVector *bv = bv_create_capa(1000);
 *   bv_set_fast(bv, 900);
 *   bv_set_fast(bv, 920);
 *   bv_set_fast(bv, 999);
 * </pre>
 *
 * While these are not ok;
 * <pre>
 *   BitVector *bv = bv_create_capa(90);
 *   bv_set_fast(bv, 80);
 *   bv_set_fast(bv, 79); // <= Bad: Out of Order
 *   bv_set_fast(bv, 80); // <= Bad: Already set
 *   bv_set_fast(bv, 90); // <= Bad: Out of Range. index must be < capa
 * </pre>
 *
 * @param bv the BitVector to set the bit in
 * @param index the index of the bit to set
 */
extern void bv_set_fast(BitVector * bv, int bit);

/**
 * Return 1 if the bit at +index+ was set or 0 otherwise. If +index+ is out of
 * range, that is greater then the BitVectors capacity, it will also return 0.
 *
 * @param bv the BitVector to check in
 * @param index the index of the bit to check
 * @return 1 if the bit was set, 0 otherwise
 */
extern int bv_get(BitVector *bv, int index);

/**
 * Unset the bit at position +index+. If the +index+ was out of range, that is
 * greater than the BitVectors capacity then do nothing. (bv_get will return 0
 * in this case anyway).
 *
 * @param bv the BitVector to unset the bit in
 * @param index the index of the bit to unset
 */
extern void bv_unset(BitVector * bv, int bit);

/**
 * Clear all set bits. This function will set all set bits to 0.
 *
 * @param bv the BitVector to clear
 */
extern void bv_clear(BitVector *bv);

/**
 * Resets the set bit count by running through the whole BitVector and
 * counting all set bits. A running count of the bits is kept by bv_set,
 * bv_get and bv_set_fast so this function is only necessary if the count could
 * have been corrupted somehow or if the BitVector has been constructed in a
 * different way (for example being read from the file_system).
 *
 * @param bv the BitVector to count the bits in
 * @return the number of set bits in the BitVector. BitVector.count is also
 *   set
 */
extern int bv_recount(BitVector * bv);

/**
 * Reset the BitVector for scanning. This function should be called before
 * using bv_scan_next to scan through all set bits in the BitVector. This is
 * not necessary when using bv_scan_next_from.
 *
 * @param bv the BitVector to reset for scanning
 */
extern void bv_scan_reset(BitVector * bv);

/**
 * Scan the BitVector for the next set bit. Before using this function you
 * should reset the BitVector for scanning using +bv_scan_reset+. You can the
 * repeated call bv_scan_next to get each set bit until it finally returns
 * -1.
 *
 * @param bv the BitVector to scan
 * @return the next set bits index or -1 if no more bits are set
 */
extern int bv_scan_next(BitVector * bv);

/**
 * Scan the BitVector for the next set bit after +from+. If no more bits are
 * set then return -1, otherwise return the index of teh next set bit.
 *
 * @param bv the BitVector to scan
 * @return the next set bit's index or -1 if no more bits are set
 */
extern int bv_scan_next_from(BitVector * bv, register const int from);

#endif
