#ifndef FRT_BIT_VECTOR_H
#define FRT_BIT_VECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "global.h"

#define FRT_BV_INIT_CAPA 256

typedef struct FrtBitVector
{
    /** The bits are held in an array of 32-bit integers */
    frt_u32 *bits;

    /** size is equal to 1 + the highest order bit set */
    int size;

    /** capa is the number of words (U32) allocated for the bits */
    int capa;

    /** count is the running count of bits set. This is kept up to
     * date by frt_bv_set and frt_bv_unset. You can reset this value
     * by calling frt_bv_recount */
    int count;

    /** curr_bit is used by scan_next to record the previously  scanned bit */
    int curr_bit;

    bool extends_as_ones : 1;
    int ref_cnt;
} FrtBitVector;

/**
 * Create a new FrtBitVector with a capacity of
 * +FRT_BV_INIT_CAPA+. Note that the FrtBitVector is growable and will
 * adjust it's capacity when you use frt_bv_set.
 *
 * @return FrtBitVector with a capacity of +FRT_BV_INIT_CAPA+.
 */
extern FRT_ATTR_MALLOC
FrtBitVector *frt_bv_new();

/**
 * Create a new FrtBitVector with a capacity of +capa+. Note that the
 * FrtBitVector is growable and will adjust it's capacity when you use
 * frt_bv_set.
 *
 * @param capa the initial capacity of the FrtBitVector
 * @return FrtBitVector with a capacity of +capa+.
 */
extern FRT_ATTR_MALLOC
FrtBitVector *frt_bv_new_capa(int capa);

/**
 * Destroy a FrtBitVector, freeing all memory allocated to that
 * FrtBitVector
 *
 * @param bv FrtBitVector to destroy
 */
extern void frt_bv_destroy(FrtBitVector *bv);

/**
 * Set the bit at position +index+ with +value+. If +index+ is outside
 * of the range of the FrtBitVector, that is >= FrtBitVector.size,
 * FrtBitVector.size will be set to +index+ + 1. If it is greater than
 * the capacity of the FrtBitVector, the capacity will be expanded to
 * accomodate.
 *
 * @param bv the FrtBitVector to set the bit in
 * @param index the index of the bit to set
 * @param value the boolean value
 */

/*
 * FIXME: if the top set bit is unset, size is not adjusted. This will not
 * cause any bugs in this code but could cause problems if users are relying
 * on the fact that size is accurate.
 */
static FRT_ATTR_ALWAYS_INLINE
void frt_bv_set_value(FrtBitVector *bv, int bit, bool value)
{
    frt_u32 *word_p;
    int word = bit >> 5;
    frt_u32 bitmask = 1 << (bit & 31);

    /* Check to see if we need to grow the BitVector */
    if (unlikely(bit >= bv->size)) {
        bv->size = bit + 1; /* size is max range of bits set */
        if (word >= bv->capa) {
            int capa = bv->capa << 1;
            while (capa <= word) {
                capa <<= 1;
            }
            FRT_REALLOC_N(bv->bits, frt_u32, capa);
            memset(bv->bits + bv->capa, (bv->extends_as_ones ? 0xFF : 0),
                   sizeof(frt_u32) * (capa - bv->capa));
            bv->capa = capa;
        }
    }

    /* Set the required bit */
    word_p = &(bv->bits[word]);
    if ((!!(bitmask & *word_p)) != value) {
        if (value) {
            bv->count++;
            *word_p |= bitmask;
        }
        else {
            bv->count--;
            *word_p &= ~bitmask;
        }
    }
}

/**
 * Set the bit at position +index+. If +index+ is outside of the range
 * of the FrtBitVector, that is >= FrtBitVector.size,
 * FrtBitVector.size will be set to +index+ + 1. If it is greater than
 * the capacity of the FrtBitVector, the capacity will be expanded to
 * accomodate.
 *
 * @param bv the FrtBitVector to set the bit in
 * @param index the index of the bit to set
 */
static FRT_ATTR_ALWAYS_INLINE
void frt_bv_set(FrtBitVector *bv, int bit)
{
    frt_bv_set_value(bv, bit, 1);
}

/**
 * Unsafely set the bit at position +index+. If you choose to use this
 * function you must create the FrtBitVector with a large enough
 * capacity to accomodate all of the frt_bv_set_fast operations. You
 * must also set bits in order and only one time per bit. Otherwise,
 * use the safe frt_bv_set function.
 *
 * So this is ok;
 * <pre>
 *   FrtBitVector *bv = frt_bv_new_capa(1000);
 *   frt_bv_set_fast(bv, 900);
 *   frt_bv_set_fast(bv, 920);
 *   frt_bv_set_fast(bv, 999);
 * </pre>
 *
 * While these are not ok;
 * <pre>
 *   FrtBitVector *bv = frt_bv_new_capa(90);
 *   frt_bv_set_fast(bv, 80);
 *   frt_bv_set_fast(bv, 79); // <= Bad: Out of Order
 *   frt_bv_set_fast(bv, 80); // <= Bad: Already set
 *   frt_bv_set_fast(bv, 90); // <= Bad: Out of Range. index must be < capa
 * </pre>
 *
 * @param bv the FrtBitVector to set the bit in
 * @param index the index of the bit to set
 */
static FRT_ATTR_ALWAYS_INLINE
void frt_bv_set_fast(FrtBitVector *bv, int bit)
{
    bv->count++;
    bv->size = bit + 1;
    bv->bits[bit >> 5] |= (1 << (bit & 31));
}

/**
 * Return 1 if the bit at +index+ was set or 0 otherwise. If +index+
 * is out of range, that is greater then the BitVectors capacity, it
 * will also return 0.
 *
 * @param bv the FrtBitVector to check in
 * @param index the index of the bit to check
 * @return 1 if the bit was set, 0 otherwise
 */
static FRT_ATTR_ALWAYS_INLINE
int frt_bv_get(FrtBitVector *bv, int bit)
{
    /* out of range so return 0 because it can't have been set */
    if (unlikely(bit >= bv->size)) {
        return bv->extends_as_ones;
    }
    return (bv->bits[bit >> 5] >> (bit & 31)) & 0x01;
}

/**
 * Unset the bit at position +index+. If the +index+ was out of range,
 * that is greater than the BitVectors capacity then do
 * nothing. (frt_bv_get will return 0 in this case anyway).
 *
 * @param bv the FrtBitVector to unset the bit in
 * @param index the index of the bit to unset
 */
static FRT_ATTR_ALWAYS_INLINE
void frt_bv_unset(FrtBitVector *bv, int bit)
{
    frt_bv_set_value(bv, bit, 0);
}

/**
 * Clear all set bits. This function will set all set bits to 0.
 *
 * @param bv the FrtBitVector to clear
 */
extern void frt_bv_clear(FrtBitVector *bv);

/**
 * Resets the set bit count by running through the whole FrtBitVector
 * and counting all set bits. A running count of the bits is kept by
 * frt_bv_set, *frt_bv_get and frt_bv_set_fast so this function is
 * only necessary if the count could have been corrupted somehow or if
 * the FrtBitVector has been constructed in a different way (for
 * example being read from the file_system).
 *
 * @param bv the FrtBitVector to count the bits in
 * @return the number of set bits in the FrtBitVector. FrtBitVector.count is also
 *   set
 */
static FRT_ATTR_ALWAYS_INLINE
int frt_bv_recount(FrtBitVector *bv)
{
    unsigned int extra = ((bv->size & 31) >> 3) + 1;
    unsigned int len   = bv->size >> 5;
    unsigned int idx, count = 0;

    if (bv->extends_as_ones) {
        for (idx = 0; idx < len; ++idx) {
            count += frt_count_zeros(bv->bits[idx]);
        }
        switch (extra) {
            case 4: count += frt_count_zeros(bv->bits[idx] | 0x00ffffff);
            case 3: count += frt_count_zeros(bv->bits[idx] | 0xff00ffff);
            case 2: count += frt_count_zeros(bv->bits[idx] | 0xffff00ff);
            case 1: count += frt_count_zeros(bv->bits[idx] | 0xffffff00);
        }
    }
    else {
        for (idx = 0; idx < len; ++idx) {
            count += frt_count_ones(bv->bits[idx]);
        }
        switch (extra) {
            case 4: count += frt_count_ones(bv->bits[idx] & 0xff000000);
            case 3: count += frt_count_ones(bv->bits[idx] & 0x00ff0000);
            case 2: count += frt_count_ones(bv->bits[idx] & 0x0000ff00);
            case 1: count += frt_count_ones(bv->bits[idx] & 0x000000ff);
        }
    }
    return bv->count = count;
}

/**
 * Reset the FrtBitVector for scanning. This function should be called
 * before using frt_bv_scan_next to scan through all set bits in the
 * FrtBitVector. This is not necessary when using
 * frt_bv_scan_next_from.
 *
 * @param bv the FrtBitVector to reset for scanning
 */
extern void frt_bv_scan_reset(FrtBitVector *bv);

/**
 * Scan the FrtBitVector for the next set bit after +from+. If no more
 * bits are set then return -1, otherwise return the index of teh next
 * set bit.
 *
 * @param bv the FrtBitVector to scan
 * @return the next set bit's index or -1 if no more bits are set
 */
static FRT_ATTR_ALWAYS_INLINE
int frt_bv_scan_next_from(FrtBitVector *bv, const int bit)
{
    frt_u32 pos  = bit >> 5;
    frt_u32 word = bv->bits[pos];

    if (bit >= bv->size)
        return -1;

    /* Keep only the bits above this position */
    word &= ~0 << (bit & 31);
    if (word) {
        goto done;
    }
    else {
        frt_u32 word_size = FRT_TO_WORD(bv->size);
        for (pos++; pos < word_size; ++pos)
        {
            if ( (word = bv->bits[pos]) )
                goto done;
        }
    }
        return -1;
 done:
    return bv->curr_bit = (pos << 5) + frt_count_trailing_zeros(word);
}

/**
 * Scan the FrtBitVector for the next set bit. Before using this
 * function you should reset the FrtBitVector for scanning using
 * +frt_bv_scan_reset+. You can the repeatedly call frt_bv_scan_next
 * to get each set bit until it finally returns -1.
 *
 * @param bv the FrtBitVector to scan
 * @return the next set bits index or -1 if no more bits are set
 */
static FRT_ATTR_ALWAYS_INLINE
int frt_bv_scan_next(FrtBitVector *bv)
{
    return frt_bv_scan_next_from(bv, bv->curr_bit + 1);
}

/**
 * Scan the FrtBitVector for the next unset bit after +from+. If no
 * more bits are unset then return -1, otherwise return the index of
 * teh next unset bit.
 *
 * @param bv the FrtBitVector to scan
 * @return the next unset bit's index or -1 if no more bits are unset
 */
static FRT_ATTR_ALWAYS_INLINE
int frt_bv_scan_next_unset_from(FrtBitVector *bv, const int bit)
{
    frt_u32 pos  = bit >> 5;
    frt_u32 word = bv->bits[pos];

    if (bit >= bv->size)
        return -1;

    /* Set all of the bits below this position */
    word |= (1 << (bit & 31)) - 1;
    if (~word) {
        goto done;
    }
    else {
        frt_u32 word_size = FRT_TO_WORD(bv->size);
        for (pos++; pos < word_size; ++pos)
        {
            if ( ~(word = bv->bits[pos]) )
                goto done;
        }
    }
    return -1;
 done:
    return bv->curr_bit = (pos << 5) + frt_count_trailing_ones(word);
}

/**
 * Scan the FrtBitVector for the next unset bit. Before using this
 * function you should reset the FrtBitVector for scanning using
 * +frt_bv_scan_reset+. You can the repeated call frt_bv_scan_next to
 * get each unset bit until it finally returns -1.
 *
 * @param bv the FrtBitVector to scan
 * @return the next unset bits index or -1 if no more bits are unset
 */
static FRT_ATTR_ALWAYS_INLINE
int frt_bv_scan_next_unset(FrtBitVector *bv)
{
    return frt_bv_scan_next_unset_from(bv, bv->curr_bit + 1);
}

/**
 * Check whether the two BitVectors have the same bits set.
 *
 * @param bv1 first FrtBitVector to compare
 * @param bv2 second BitVectors to compare
 * @return true if bv1 == bv2
 */
extern int frt_bv_eq(FrtBitVector *bv1, FrtBitVector *bv2);

/**
 * Determines a hash value for the FrtBitVector
 *
 * @param bv the FrtBitVector to hash
 * @return A hash value for the FrtBitVector
 */
extern unsigned long frt_bv_hash(FrtBitVector *bv);

static FRT_ATTR_ALWAYS_INLINE
void frt_bv_capa(FrtBitVector *bv, int capa, int size)
{
    int word_size = FRT_TO_WORD(size);
    if (bv->capa < capa)
    {
        FRT_REALLOC_N(bv->bits, frt_u32, capa);
        bv->capa = capa;
        memset(bv->bits + word_size, (bv->extends_as_ones ? 0xFF : 0),
               sizeof(frt_u32) * (capa - word_size));
    }
    bv->size = size;
}

#define frt_bv_and_ext(dest, src, extends_as_ones, i, max) do { \
    if (extends_as_ones)                                        \
         memcpy(&dest[i], &src[i], sizeof(*dest)*(max - i));    \
    else memset(&dest[i], 0x00   , sizeof(*dest)*(max - i));    \
} while(0)

#define frt_bv_or_ext(dest, src, extends_as_ones, i, max) do { \
    if (extends_as_ones)                                       \
         memset(&dest[i], 0xFF   , sizeof(*dest)*(max - i));   \
    else memcpy(&dest[i], &src[i], sizeof(*dest)*(max - i));   \
} while(0)

#define frt_bv_xor_ext(dest, src, extends_as_ones, i, max) do { \
    frt_u32 n = (extends_as_ones ? 0xffffffff : 0);             \
    for (; i < max; ++i)                                        \
        dest[i] = src[i] ^ n;                                   \
} while(0)

#define FRT_BV_OP(bv, a, b, op, ext_cb) do {                          \
    int i;                                                            \
    int a_wsz = FRT_TO_WORD(a->size);                                 \
    int b_wsz = FRT_TO_WORD(b->size);                                 \
    int max_size = frt_max2(a->size, b->size);                        \
    int min_size = frt_min2(a->size, b->size);                        \
    int max_word_size = FRT_TO_WORD(max_size);                        \
    int min_word_size = FRT_TO_WORD(min_size);                        \
    int capa = frt_max2(frt_round2(max_word_size), 4);                \
                                                                      \
    bv->extends_as_ones = (a->extends_as_ones op b->extends_as_ones); \
    frt_bv_capa(bv, capa, max_size);                                  \
                                                                      \
    for (i = 0; i < min_word_size; ++i)                               \
        bv->bits[i] = a->bits[i] op b->bits[i];                       \
                                                                      \
    if (a_wsz != b_wsz) {                                             \
        frt_u32 *bits = a->bits;                                      \
        bool extends_as_ones = b->extends_as_ones;                    \
        if (a_wsz < b_wsz) {                                          \
            bits = b->bits;                                           \
            extends_as_ones = a->extends_as_ones;                     \
        }                                                             \
        ext_cb(bv->bits, bits, extends_as_ones, i, max_word_size);    \
    }                                                                 \
    frt_bv_recount(bv);                                               \
} while(0)

static FRT_ATTR_ALWAYS_INLINE
FrtBitVector *frt_bv_and_i(FrtBitVector *bv,
                           FrtBitVector *a, FrtBitVector *b)
{
    FRT_BV_OP(bv, a, b, &, frt_bv_and_ext);
    return bv;
}

static FRT_ATTR_ALWAYS_INLINE
FrtBitVector *frt_bv_or_i(FrtBitVector *bv,
                          FrtBitVector *a, FrtBitVector *b)
{
    FRT_BV_OP(bv, a, b, |, frt_bv_or_ext);
    return bv;
}

static FRT_ATTR_ALWAYS_INLINE
FrtBitVector *frt_bv_xor_i(FrtBitVector *bv,
                           FrtBitVector *a, FrtBitVector *b)
{
    FRT_BV_OP(bv, a, b, ^, frt_bv_xor_ext);
    return bv;
}

static FRT_ATTR_ALWAYS_INLINE
FrtBitVector *frt_bv_not_i(FrtBitVector *bv, FrtBitVector *bv1)
{
    int i;
    int word_size = FRT_TO_WORD(bv1->size);
    int capa = frt_max2(frt_round2(word_size), 4);

    bv->extends_as_ones = !bv1->extends_as_ones;
    frt_bv_capa(bv, capa, bv1->size);

    for (i = 0; i < word_size; i++)
        bv->bits[i] = ~(bv1->bits[i]);

    memset(bv->bits + word_size, (bv->extends_as_ones ? 0xFF : 0),
           sizeof(frt_u32) * (bv->capa - word_size));

    frt_bv_recount(bv);
    return bv;
}

/**
 * ANDs two BitVectors (+bv1+ and +bv2+) together and return the resultant
 * FrtBitVector
 *
 * @param bv1 first FrtBitVector to AND
 * @param bv2 second FrtBitVector to AND
 * @return A FrtBitVector with all bits set that are set in both bv1 and bv2
 */
static FRT_ATTR_ALWAYS_INLINE
FrtBitVector *frt_bv_and(FrtBitVector *bv1, FrtBitVector *bv2)
{
    return frt_bv_and_i(frt_bv_new(), bv1, bv2);
}

/**
 * ORs two BitVectors (+bv1+ and +bv2+) together and return the resultant
 * FrtBitVector
 *
 * @param bv1 first FrtBitVector to OR
 * @param bv2 second FrtBitVector to OR
 * @return A FrtBitVector with all bits set that are set in both bv1 and bv2
 */
static FRT_ATTR_ALWAYS_INLINE
FrtBitVector *frt_bv_or(FrtBitVector *bv1, FrtBitVector *bv2)
{
    return frt_bv_or_i(frt_bv_new(), bv1, bv2);
}


/**
 * XORs two BitVectors (+bv1+ and +bv2+) together and return the resultant
 * FrtBitVector
 *
 * @param bv1 first FrtBitVector to XOR
 * @param bv2 second FrtBitVector to XOR
 * @return A FrtBitVector with all bits set that are equal in bv1 and bv2
 */
static FRT_ATTR_ALWAYS_INLINE
FrtBitVector *frt_bv_xor(FrtBitVector *bv1, FrtBitVector *bv2)
{
    return frt_bv_xor_i(frt_bv_new(), bv1, bv2);
}

/**
 * Returns FrtBitVector with all of +bv+'s bits flipped
 *
 * @param bv FrtBitVector to flip
 * @return A FrtBitVector with all bits set that are set in both bv1 and bv2
 */
static FRT_ATTR_ALWAYS_INLINE
FrtBitVector *frt_bv_not(FrtBitVector *bv)
{
    return frt_bv_not_i(frt_bv_new(), bv);
}

/**
 * ANDs two BitVectors together +bv1+ and +bv2+ in place of +bv1+
 *
 * @param bv1 first FrtBitVector to AND
 * @param bv2 second FrtBitVector to AND
 * @return A FrtBitVector
 * @return bv1 with all bits set that where set in both bv1 and bv2
 */
static FRT_ATTR_ALWAYS_INLINE
FrtBitVector *frt_bv_and_x(FrtBitVector *bv1, FrtBitVector *bv2)
{
    return frt_bv_and_i(bv1, bv1, bv2);
}

/**
 * ORs two BitVectors together
 *
 * @param bv1 first FrtBitVector to OR
 * @param bv2 second FrtBitVector to OR
 * @return bv1
 */
static FRT_ATTR_ALWAYS_INLINE
FrtBitVector *frt_bv_or_x(FrtBitVector *bv1, FrtBitVector *bv2)
{
    return frt_bv_or_i(bv1, bv1, bv2);
}

/**
 * XORs two BitVectors together +bv1+ and +bv2+ in place of +bv1+
 *
 * @param bv1 first FrtBitVector to XOR
 * @param bv2 second FrtBitVector to XOR
 * @return bv1
 */
static FRT_ATTR_ALWAYS_INLINE
FrtBitVector *frt_bv_xor_x(FrtBitVector *bv1, FrtBitVector *bv2)
{
    return frt_bv_xor_i(bv1, bv1, bv2);
}

/**
 * Flips all bits in the FrtBitVector +bv+
 *
 * @param bv FrtBitVector to flip
 * @return A +bv+ with all it's bits flipped
 */
static FRT_ATTR_ALWAYS_INLINE
FrtBitVector *frt_bv_not_x(FrtBitVector *bv)
{
    return frt_bv_not_i(bv, bv);
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif
