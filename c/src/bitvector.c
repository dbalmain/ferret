#include "bitvector.h"
#include "internal.h"
#include <string.h>

BitVector *bv_new_capa(int capa)
{
    BitVector *bv = ALLOC_AND_ZERO(BitVector);

    /* The capacity passed by the user is number of bits allowed, however we
     * store capacity as the number of words (U32) allocated. */
    bv->capa = max2(TO_WORD(capa), 4);
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

void bv_scan_reset(BitVector *bv)
{
    bv->curr_bit = -1;
}

int bv_eq(BitVector *bv1, BitVector *bv2)
{
    if (bv1 == bv2) {
        return true;
    }

    if (bv1->extends_as_ones != bv2->extends_as_ones) {
        return false;
    }

    u32 *bits = bv1->bits;
    u32 *bits2 = bv2->bits;
    int min_size = min2(bv1->size, bv2->size);
    int word_size = TO_WORD(min_size);
    int ext_word_size = 0;
    int i;

    for (i = 0; i < word_size; i++) {
        if (bits[i] != bits2[i]) {
            return false;
        }
    }
    if (bv1->size > min_size) {
        bits = bv1->bits;
        ext_word_size = TO_WORD(bv1->size);
    }
    else if (bv2->size > min_size) {
        bits = bv2->bits;
        ext_word_size = TO_WORD(bv2->size);
    }
    if (ext_word_size) {
        const u32 expected = (bv1->extends_as_ones ? 0xFFFFFFFF : 0);
        for (i = word_size; i < ext_word_size; i++) {
            if (bits[i] != expected) {
                return false;
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
    for (i = TO_WORD(bv->size) - 1; i >= 0; i--) {
        const u32 word = bv->bits[i];
        if (word != empty_word)
            hash = (hash << 1) ^ word;
    }
    return (hash << 1) | bv->extends_as_ones;
}
