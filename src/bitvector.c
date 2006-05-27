#include "bitvector.h"
#include <string.h>

BitVector *bv_create_capa(int capa)
{
    BitVector *bv = ALLOC(BitVector);

    /* The capacity passed by the user is number of bits allowed, however we
     * store capacity as the number of words (U32) allocated. */
    bv->capa = (capa >> 5) + 1;
    bv->bits = ALLOC_AND_ZERO_N(f_u32, bv->capa);

    bv->size = 0;
    bv->count = 0;
    bv->curr_bit = -1;
    return bv;
}

BitVector *bv_create()
{
    return bv_create_capa(BV_INIT_CAPA);
}

void bv_destroy(BitVector * bv)
{
    free(bv->bits);
    free(bv);
}

void bv_set(BitVector * bv, int bit)
{
    f_u32 *word_p;
    int word = bit >> 5;
    f_u32 bitmask = 1 << (bit & 31);
    
    /* Check to see if we need to grow the BitVector */
    if (bit > bv->size) {
        bv->size = bit + 1; /* size is max range of bits set */
        if (word >= bv->capa) {
            int capa = bv->capa << 1;
            while (capa <= word) {
                capa <<= 1;
            }
            REALLOC_N(bv->bits, f_u32, capa);
            memset(bv->bits + bv->capa, 0, sizeof(f_u32) * (capa - bv->capa));
            bv->capa = capa;
        }
    }
    
    /* Set the required bit */
    word_p = &(bv->bits[word]);
    if ((bitmask & *word_p) == 0) {
        bv->count++; /* update count */
        *word_p |= bitmask;
    }
}

/*
 * This method relies on the fact that enough space has been set for the bits
 * to be set. You need to create the BitVector using bv_create_capa(capa) with
 * a capacity larger than any bit being set.
 */
void bv_set_fast(BitVector * bv, int bit)
{
    bv->count++;
    bv->size = bit;
    bv->bits[bit >> 5] |= 1 << (bit & 31);
}

int bv_get(BitVector * bv, int bit)
{
    /* out of range so return 0 because it can't have been set */
    if (bit >= bv->size) {
        return 0;
    }
    return (bv->bits[bit >> 5] >> (bit & 31)) & 0x01;
}

void bv_clear(BitVector * bv)
{
    memset(bv->bits, 0, bv->capa * sizeof(f_u32));
    bv->count = 0;
    bv->size = 0;
}

/*
 * FIXME: if the top set bit is unset, size is not adjusted. This will not
 * cause any bugs in this code but could cause problems if users are relying
 * on the fact that size is accurate.
 */
void bv_unset(BitVector * bv, int bit)
{
    f_u32 *word_p;
    f_u32 bitmask;
    int word = bit >> 5;

    /* out of range so no need to set it. The out of range values are assumed
     * to be unset */
    if (bit >= bv->size) {
        return;
    }

    word_p = &(bv->bits[word]);
    bitmask = 1 << (bit & 31);
    if ((bitmask & *word_p) > 0) {
        bv->count--; /* update count */
        *word_p &= ~bitmask;
    }
}

/* Table of bits per char. This table is used by the bv_recount method to
 * optimize the counting of bits */
const uchar BYTE_COUNTS[] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

int bv_recount(BitVector * bv)
{
    /* if the vector has been modified */
    int i, c = 0;
    uchar *bytes = (uchar *)bv->bits; /* count by character */
    for (i = ((bv->size >> 5) + 1) << 2; i >= 0; i--) {
        c += BYTE_COUNTS[bytes[i]];     /* sum bits per char */
    }
    bv->count = c;
    return c;
}

void bv_scan_reset(BitVector * bv)
{
    bv->curr_bit = -1;
}

/* Table showing the number of trailing 0s in a char. This is used to optimize
 * the bv_scan_next method.  */
const int NUM_TRAILING_ZEROS[] = {
    8, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
};

/*
 * This method is highly optimized, hence the loop unrolling
 */
int bv_scan_next_from(BitVector * bv, register const int from)
{
    register const f_u32 *const bits = bv->bits;
    register const int word_size = (bv->size >> 5) + 1;
    register int word_pos = from >> 5;
    register int bit_pos = (from & 31);
    register int word = bits[word_pos] >> bit_pos;

    if (from >= bv->size) {
        return -1;
    }
    if (word == 0) {
        bit_pos = 0;
        do {
            word_pos++;
            if (word_pos >= word_size) {
                return -1;
            }
        } while (bits[word_pos] == 0);
        word = bits[word_pos];
    }

    /* check the word a byte at a time as the NUM_TRAILING_ZEROS table would
     * be too large for 32-bit integer or even a 16-bit integer */
    if (word & 0xff) {
      bit_pos += NUM_TRAILING_ZEROS[word & 0xff];
    }
    else {
      word >>= 8;
      if (word & 0xff) {
        bit_pos += NUM_TRAILING_ZEROS[word & 0xff] + 8;
      }
      else {
        word >>= 8;
        if (word & 0xff) {
          bit_pos += NUM_TRAILING_ZEROS[word & 0xff] + 16;
        }
        else {
          word >>= 8;
          bit_pos += NUM_TRAILING_ZEROS[word & 0xff] + 24;
        }
      }
    }

    /*
     * second fastest;
     *
     *   while ((inc = NUM_TRAILING_ZEROS[word & 0xff]) == 8) {
     *       word >>= 8;
     *       bit_pos += 8;
     *   }
     *
     * third fastest;
     *
     *   bit_pos += inc;
     *   if ((word & 0xffff) == 0) {
     *     bit_pos += 16;
     *     word >>= 16;
     *   }
     *   if ((word & 0xff) == 0) {
     *     bit_pos += 8;
     *     word >>= 8;
     *   }
     *   bit_pos += NUM_TRAILING_ZEROS[word & 0xff];
     */

    return bv->curr_bit = ((word_pos << 5) + bit_pos);
}

int bv_scan_next(BitVector * bv)
{
    return bv_scan_next_from(bv, bv->curr_bit + 1);
}
