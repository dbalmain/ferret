#include "hash.h"
#include "global.h"
#include <string.h>
#include "internal.h"

/****************************************************************************
 *
 * Hash
 *
 * This hash table is modeled after Python's dictobject and a description of
 * the algorithm can be found in the file dictobject.c in Python's src
 ****************************************************************************/

static char *dummy_key = "";
static char *dummy_int_key = "i";


#define PERTURB_SHIFT 5
#define MAX_FREE_HASH_TABLES 80

static Hash *free_hts[MAX_FREE_HASH_TABLES];
static int num_free_hts = 0;

unsigned long str_hash(const char *const str)
{
    register unsigned long h = 0;
    register unsigned char *p = (unsigned char *)str;

    for (; *p; p++) {
        h = 37 * h + *p;
    }

    return h;
}

unsigned long ptr_hash(const void *const ptr)
{
    return (unsigned long)ptr;
}

int ptr_eq(const void *q1, const void *q2)
{
    return q1 == q2;
}

static int str_eq(const void *q1, const void *q2)
{
    return strcmp((const char *)q1, (const char *)q2) == 0;
}

typedef HashEntry *(*lookup_ft)(struct Hash *self, register const void *key);

/**
 * Fast lookup function for resizing as we know there are no equal elements or
 * deletes to worry about.
 *
 * @param self the Hash to do the fast lookup in
 * @param the hashkey we are looking for
 */
static INLINE HashEntry *h_resize_lookup(Hash *self,
                                         register const unsigned long hash)
{
    register unsigned long perturb;
    register int mask = self->mask;
    register HashEntry *he0 = self->table;
    register int i = hash & mask;
    register HashEntry *he = &he0[i];

    if (he->key == NULL) {
        he->hash = hash;
        return he;
    }

    for (perturb = hash;; perturb >>= PERTURB_SHIFT) {
        i = (i << 2) + i + perturb + 1;
        he = &he0[i & mask];
        if (he->key == NULL) {
            he->hash = hash;
            return he;
        }
    }
}

static HashEntry *h_lookup_ptr(Hash *self, const void *key)
{
    register const unsigned long hash = (long)key;
    register unsigned long perturb;
    register int mask = self->mask;
    register HashEntry *he0 = self->table;
    register int i = hash & mask;
    register HashEntry *he = &he0[i];
    register HashEntry *freeslot = NULL;

    if (he->key == NULL || he->hash == hash) {
        he->hash = hash;
        return he;
    }
    if (he->key == dummy_key) {
        freeslot = he;
    }

    for (perturb = hash;; perturb >>= PERTURB_SHIFT) {
        i = (i << 2) + i + perturb + 1;
        he = &he0[i & mask];
        if (he->key == NULL) {
            if (freeslot != NULL) {
                he = freeslot;
            }
            he->hash = hash;
            return he;
        }
        if (he->hash == hash) {
            return he;
        }
        if (he->key == dummy_key && freeslot == NULL) {
            freeslot = he;
        }
    }
}

HashEntry *h_lookup(Hash *self, register const void *key)
{
    register const unsigned long hash = self->hash_i(key);
    register unsigned long perturb;
    register int mask = self->mask;
    register HashEntry *he0 = self->table;
    register int i = hash & mask;
    register HashEntry *he = &he0[i];
    register HashEntry *freeslot = NULL;
    eq_ft eq = self->eq_i;

    if (he->key == NULL || he->key == key) {
        he->hash = hash;
        return he;
    }
    if (he->key == dummy_key) {
        freeslot = he;
    }
    else {
        if ((he->hash == hash) && eq(he->key, key)) {
            return he;
        }
    }

    for (perturb = hash;; perturb >>= PERTURB_SHIFT) {
        i = (i << 2) + i + perturb + 1;
        he = &he0[i & mask];
        if (he->key == NULL) {
            if (freeslot != NULL) {
                he = freeslot;
            }
            he->hash = hash;
            return he;
        }
        if (he->key == key
            || (he->hash == hash
                && he->key != dummy_key && eq(he->key, key))) {
            return he;
        }
        if (he->key == dummy_key && freeslot == NULL) {
            freeslot = he;
        }
    }
}

Hash *h_new_str(free_ft free_key, free_ft free_value)
{
    Hash *self;
    if (num_free_hts > 0) {
        self = free_hts[--num_free_hts];
    }
    else {
        self = ALLOC(Hash);
    }
    self->fill = 0;
    self->size = 0;
    self->mask = HASH_MINSIZE - 1;
    self->table = self->smalltable;
    memset(self->smalltable, 0, sizeof(self->smalltable));
    self->lookup_i = (lookup_ft)&h_lookup;
    self->eq_i = str_eq;
    self->hash_i = (hash_ft)str_hash;

    self->free_key_i = free_key != NULL ? free_key : &dummy_free;
    self->free_value_i = free_value != NULL ? free_value : &dummy_free;
    self->ref_cnt = 1;
    return self;
}

Hash *h_new_int(free_ft free_value)
{
    Hash *self     = h_new_str(NULL, free_value);

    self->lookup_i = &h_lookup_ptr;
    self->eq_i     = NULL;
    self->hash_i   = NULL;

    return self;
}

Hash *h_new(hash_ft hash, eq_ft eq, free_ft free_key, free_ft free_value)
{
    Hash *self     = h_new_str(free_key, free_value);

    self->lookup_i = &h_lookup;
    self->eq_i     = eq;
    self->hash_i   = hash;

    return self;
}

void h_clear(Hash *self)
{
    int i;
    HashEntry *he;
    free_ft free_key   = self->free_key_i;
    free_ft free_value = self->free_value_i;

    /* Clear all the hash values and keys as necessary */
    if (free_key != dummy_free || free_value != dummy_free) {
        for (i = 0; i <= self->mask; i++) {
            he = &self->table[i];
            if (he->key != NULL && he->key != dummy_key) {
                free_value(he->value);
                free_key(he->key);
            }
            he->key = NULL;
        }
    }
    ZEROSET_N(self->table, HashEntry, self->mask + 1);
    self->size = 0;
    self->fill = 0;
}

void h_destroy(Hash *self)
{
    if (--(self->ref_cnt) <= 0) {
        h_clear(self);

        /* if a new table was created, be sure to free it */
        if (self->table != self->smalltable) {
            free(self->table);
        }

        if (num_free_hts < MAX_FREE_HASH_TABLES) {
            free_hts[num_free_hts++] = self;
        }
        else {
            free(self);
        }
    }
}

void *h_get(Hash *self, const void *key)
{
    /* Note: lookup_i will never return NULL. */
    return self->lookup_i(self, key)->value;
}

int h_del(Hash *self, const void *key)
{
    HashEntry *he = self->lookup_i(self, key);

    if (he->key != NULL && he->key != dummy_key) {
        self->free_key_i(he->key);
        self->free_value_i(he->value);
        he->key = dummy_key;
        he->value = NULL;
        self->size--;
        return true;
    }
    else {
        return false;
    }
}

void *h_rem(Hash *self, const void *key, bool destroy_key)
{
    void *val;
    HashEntry *he = self->lookup_i(self, key);

    if (he->key != NULL && he->key != dummy_key) {
        if (destroy_key) {
            self->free_key_i(he->key);
        }

        he->key = dummy_key;
        val = he->value;
        he->value = NULL;
        self->size--;
        return val;
    }
    else {
        return NULL;
    }
}

static int h_resize(Hash *self, int min_newsize)
{
    HashEntry smallcopy[HASH_MINSIZE];
    HashEntry *oldtable;
    HashEntry *he_old, *he_new;
    int newsize, num_active;

    /* newsize will be a power of two */
    for (newsize = HASH_MINSIZE; newsize < min_newsize; newsize <<= 1) {
    }

    oldtable = self->table;
    if (newsize == HASH_MINSIZE) {
        if (self->table == self->smalltable) {
            /* need to copy the data out so we can rebuild the table into
             * the same space */
            memcpy(smallcopy, self->smalltable, sizeof(smallcopy));
            oldtable = smallcopy;
        }
        else {
            self->table = self->smalltable;
        }
    }
    else {
        self->table = ALLOC_N(HashEntry, newsize);
    }
    memset(self->table, 0, sizeof(HashEntry) * newsize);
    self->fill = self->size;
    self->mask = newsize - 1;

    for (num_active = self->size, he_old = oldtable; num_active > 0; he_old++) {
        if (he_old->key && he_old->key != dummy_key) {    /* active entry */
            /*he_new = self->lookup_i(self, he_old->key); */
            he_new = h_resize_lookup(self, he_old->hash);
            he_new->key = he_old->key;
            he_new->value = he_old->value;
            num_active--;
        }                       /* else empty entry so nothing to do */
    }
    if (oldtable != smallcopy && oldtable != self->smalltable) {
        free(oldtable);
    }
    return 0;
}

INLINE bool h_set_ext(Hash *self, const void *key, HashEntry **he)
{
    *he = self->lookup_i(self, key);
    if ((*he)->key == NULL) {
        if (self->fill * 3 > self->mask * 2) {
            h_resize(self, self->size * ((self->size > SLOW_DOWN) ? 4 : 2));
            *he = self->lookup_i(self, key);
        }
        self->fill++;
        self->size++;
        return true;
    }
    else if ((*he)->key == dummy_key) {
        self->size++;
        return true;
    }

    return false;
}

HashKeyStatus h_set(Hash *self, const void *key, void *value)
{
    HashKeyStatus ret_val = HASH_KEY_DOES_NOT_EXIST;
    HashEntry *he;
    if (!h_set_ext(self, key, &he)) {
        if (he->key != key) {
            self->free_key_i(he->key);
            if (he->value != value) {
                self->free_value_i(he->value);
            }
            ret_val = HASH_KEY_EQUAL;
        }
        else {
            /* Only free old value if it isn't the new value */
            if (he->value != value) {
                self->free_value_i(he->value);
            }
            ret_val = HASH_KEY_SAME;
        }
    }
    he->key = (void *)key;
    he->value = value;

    return ret_val;
}

int h_set_safe(Hash *self, const void *key, void *value)
{
    HashEntry *he;
    if (h_set_ext(self, key, &he)) {
        he->key = (void *)key;
        he->value = value;
        return true;
    }
    else {
        return false;
    }
}

HashKeyStatus h_has_key(Hash *self, const void *key)
{
    HashEntry *he = self->lookup_i(self, key);
    if (he->key == NULL || he->key == dummy_key) {
        return HASH_KEY_DOES_NOT_EXIST;
    }
    else if (he->key == key) {
        return HASH_KEY_SAME;
    }
    return HASH_KEY_EQUAL;
}

INLINE void *h_get_int(Hash *self, const unsigned long key)
{
    return h_get(self, (const void *)key);
}

INLINE int h_del_int(Hash *self, const unsigned long key)
{
    return h_del(self, (const void *)key);
}

INLINE void *h_rem_int(Hash *self, const unsigned long key)
{
    return h_rem(self, (const void *)key, false);
}

INLINE HashKeyStatus h_set_int(Hash *self,
                               const unsigned long key,
                               void *value)
{
    HashKeyStatus ret_val = HASH_KEY_DOES_NOT_EXIST;
    HashEntry *he;
    if (!h_set_ext(self, (const void *)key, &he)) {
        /* Only free old value if it isn't the new value */
        if (he->value != value) {
            self->free_value_i(he->value);
        }
        ret_val = HASH_KEY_EQUAL;
    }
    he->key = dummy_int_key;
    he->value = value;

    return ret_val;
}

INLINE int h_set_safe_int(Hash *self, const unsigned long key, void *value)
{
    HashEntry *he;
    if (h_set_ext(self, (const void *)key, &he)) {
        he->key = dummy_int_key;
        he->value = value;
        return true;
    }
    return false;
}

INLINE int h_has_key_int(Hash *self, const unsigned long key)
{
    return h_has_key(self, (const void *)key);
}

void h_each(Hash *self,
            void (*each_kv) (void *key, void *value, void *arg), void *arg)
{
    HashEntry *he;
    int i = self->size;
    for (he = self->table; i > 0; he++) {
        if (he->key && he->key != dummy_key) {        /* active entry */
            each_kv(he->key, he->value, arg);
            i--;
        }
    }
}

Hash *h_clone(Hash *self, h_clone_ft clone_key, h_clone_ft clone_value)
{
    void *key, *value;
    HashEntry *he;
    int i = self->size;
    Hash *ht_clone;

    ht_clone = h_new(self->hash_i,
                     self->eq_i,
                     self->free_key_i,
                     self->free_value_i);

    for (he = self->table; i > 0; he++) {
        if (he->key && he->key != dummy_key) {        /* active entry */
            key = clone_key ? clone_key(he->key) : he->key;
            value = clone_value ? clone_value(he->value) : he->value;
            h_set(ht_clone, key, value);
            i--;
        }
    }
    return ht_clone;
}

void h_str_print_keys(Hash *self, FILE *out)
{
    HashEntry *he;
    int i = self->size;
    char **keys = ALLOC_N(char *, self->size);
    for (he = self->table; i > 0; he++) {
        if (he->key && he->key != dummy_key) {        /* active entry */
            i--;
            keys[i] = (char *)he->key;
        }
    }
    strsort(keys, self->size);
    fprintf(out, "keys:\n");
    for (i = 0; i < self->size; i++) {
        fprintf(out, "\t%s\n", keys[i]);
    }
    free(keys);
}

void hash_finalize()
{
    while (num_free_hts > 0) {
        free(free_hts[--num_free_hts]);
    }
}
