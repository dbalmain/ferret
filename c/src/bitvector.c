#include "bitvector.h"
#include "internal.h"
#include <string.h>

BitVector *bv_new_capa(int capa)
{
    BitVector *bv = ALLOC_AND_ZERO(BitVector);

    /* The capacity passed by the user is number of bits allowed, however we
     * store capacity as the number of words (U32) allocated. */
    bv->capa = (capa >> 5) + 1;
    bv->bits = ALLOC_AND_ZERO_N(u32, bv->capa);
    bv->curr_bit = -1;
    bv->ref_cnt = 1;
    return bv;
}

BitVector *bv_new()
{
    return bv_new_capa(BV_INIT_CAPA);
}

void bv_destroy(BitVector *bv)
{
    if (--(bv->ref_cnt) == 0) {
        free(bv->bits);
        free(bv);
    }
}

void bv_clear(BitVector *bv)
{
    memset(bv->bits, 0, bv->capa * sizeof(u32));
    bv->extends_as_ones = 0;
    bv->count = 0;
    bv->size = 0;
}

int bv_recount(BitVector *bv)
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

void bv_scan_reset(BitVector *bv)
{
    bv->curr_bit = -1;
}

int bv_eq(BitVector *bv1, BitVector *bv2)
{
    if (bv1 == bv2) {
        return true;
    }
    else if (bv1->extends_as_ones != bv2->extends_as_ones) {
        return false;
    }
    else {
        u32 *bits = bv1->bits;
        u32 *bits2 = bv2->bits;
        int min_size = min2(bv1->size, bv2->size);
        int word_size = (min_size >> 5) + 1;
        int ext_word_size = 0;

        int i;

        for (i = 0; i < word_size; i++) {
            if (bits[i] != bits2[i]) {
                return false;
            }
        }
        if (bv1->size > min_size) {
            bits = bv1->bits;
            ext_word_size = (bv1->size >> 5) + 1;
        }
        else if (bv2->size > min_size) {
            bits = bv2->bits;
            ext_word_size = (bv2->size >> 5) + 1;
        }
        if (ext_word_size) {
            const u32 expected = (bv1->extends_as_ones ? 0xFFFFFFFF : 0);
            for (i = word_size; i < ext_word_size; i++) {
                if (bits[i] != expected) {
                    return false;
                }
            }
        }
    }
    return true;
}

unsigned long bv_hash(BitVector *bv)
{
    unsigned long hash = 0;
    const u32 empty_word = bv->extends_as_ones ? 0xFFFFFFFF : 0;
    int i;
    for (i = (bv->size >> 5); i >= 0; i--) {
        const u32 word = bv->bits[i];
        if (word != empty_word) {
            hash = (hash << 1) ^ word;
        }
    }
    return (hash << 1) | bv->extends_as_ones;
}

static INLINE void bv_capa(BitVector *bv, int capa, int size)
{
    int word_size = (size >> 5) + 1;
    REALLOC_N(bv->bits, u32, capa);
    bv->capa = capa;
    bv->size = size;
    memset(bv->bits + word_size, (bv->extends_as_ones ? 0xFF : 0),
           sizeof(u32) * (capa - word_size));
}

static INLINE void bv_recapa(BitVector *bv, int new_capa)
{
    if (bv->capa < new_capa) {
        REALLOC_N(bv->bits, u32, new_capa);
        memset(bv->bits + bv->capa, (bv->extends_as_ones ? 0xFF : 0),
               sizeof(u32) * (new_capa - bv->capa));
        bv->capa = new_capa;
    }
}

static BitVector *bv_and_i(BitVector *bv, BitVector *bv1, BitVector *bv2)
{
    int i;
    int size;
    int word_size;
    int capa = 4;

    bv->extends_as_ones = (bv1->extends_as_ones & bv2->extends_as_ones);

    if (bv1->extends_as_ones || bv2->extends_as_ones)
         size = max2(bv1->size, bv2->size);
    else size = min2(bv1->size, bv2->size);

    word_size = (size >> 5) + 1;
    capa = max2(frt_round2(word_size), 4);

    bv_recapa(bv1, capa);
    bv_recapa(bv2, capa);
    bv_capa(bv, capa, size);

    for (i = 0; i < word_size; i++) {
        bv->bits[i] = bv1->bits[i] & bv2->bits[i];
    }

    bv_recount(bv);
    return bv;
}

BitVector *bv_and(BitVector *bv1, BitVector *bv2)
{
    return bv_and_i(bv_new(), bv1, bv2);
}

BitVector *bv_and_x(BitVector *bv1, BitVector *bv2)
{
    return bv_and_i(bv1, bv1, bv2);
}

static BitVector *bv_or_i(BitVector *bv, BitVector *bv1, BitVector *bv2)
{
    int i;
    int max_size = max2(bv1->size, bv2->size);
    int word_size = (max_size >> 5) + 1;
    int capa = max2(frt_round2(word_size), 4);

    bv->extends_as_ones = (bv1->extends_as_ones | bv2->extends_as_ones);
    bv_recapa(bv1, capa);
    bv_recapa(bv2, capa);
    bv_capa(bv, capa, max_size);

    for (i = 0; i < word_size; i++) {
        bv->bits[i] = bv1->bits[i] | bv2->bits[i];
    }
    bv_recount(bv);
    return bv;
}

BitVector *bv_or(BitVector *bv1, BitVector *bv2)
{
    return bv_or_i(bv_new(), bv1, bv2);
}

BitVector *bv_or_x(BitVector *bv1, BitVector *bv2)
{
    return bv_or_i(bv1, bv1, bv2);
}

static BitVector *bv_xor_i(BitVector *bv, BitVector *bv1, BitVector *bv2)
{
    int i;
    int max_size = max2(bv1->size, bv2->size);
    int word_size = (max_size >> 5) + 1;
    int capa = max2(frt_round2(word_size), 4);

    bv->extends_as_ones = (bv1->extends_as_ones ^ bv2->extends_as_ones);
    bv_recapa(bv1, capa);
    bv_recapa(bv2, capa);
    bv_capa(bv, capa, max_size);

    for (i = 0; i < word_size; i++) {
        bv->bits[i] = bv1->bits[i] ^ bv2->bits[i];
    }
    bv_recount(bv);
    return bv;
}

BitVector *bv_xor(BitVector *bv1, BitVector *bv2)
{
    return bv_xor_i(bv_new(), bv1, bv2);
}

BitVector *bv_xor_x(BitVector *bv1, BitVector *bv2)
{
    return bv_xor_i(bv1, bv1, bv2);
}

static BitVector *bv_not_i(BitVector *bv, BitVector *bv1)
{
    int i;
    int word_size = (bv1->size >> 5) + 1;
    int capa = max2(frt_round2(word_size), 4);

    bv->extends_as_ones = !bv1->extends_as_ones;
    bv_capa(bv, capa, bv1->size);

    for (i = 0; i < word_size; i++) {
        bv->bits[i] = ~(bv1->bits[i]);
    }
    bv_recount(bv);
    return bv;
}

BitVector *bv_not(BitVector *bv1)
{
    return bv_not_i(bv_new(), bv1);
}

BitVector *bv_not_x(BitVector *bv1)
{
    return bv_not_i(bv1, bv1);
}
