#if defined(__cplusplus) && defined(HAVE_SPARSE_HASH)
# include <google/dense_hash_map>
# include <google/sparse_hash_map>
#endif
#include <string.h>
#ifdef __cplusplus
extern "C" {
#endif
#include "hash.h"
#include "benchmark.h"
#ifdef __cplusplus
}
#endif

#define N 20

static void ferret_hash()
{
    int i;
    for (i = 0; i < N; i++) {
        Hash *h = h_new_str(NULL, NULL);
        const char **word;
        char buf[100];
        long res;
        for (word = WORD_LIST; *word; word++) {
            h_set(h, *word, (void *)1);
        }
        for (word = WORD_LIST; *word; word++) {
            strcpy(buf, *word);
            res = (long)h_get(h, buf);
        }
        h_destroy(h);
    }
}

#ifdef __cplusplus

#include <iostream>
#include <ext/hash_map>

using __gnu_cxx::hash_map;

struct eqstr
{
    bool operator()(const char* s1, const char* s2) const
    {
        return (s1 == s2) || (s1 && s2 && strcmp(s1, s2) == 0);
    }
};

static void stdcpp_hash()
{
    for (int i = 0; i < N; i++) {
        hash_map<const char*, int, __gnu_cxx::hash<const char*>, eqstr> dict;
        const char **word;
        char buf[100];
        long res;
        for (word = WORD_LIST; *word; word++) {
            dict[*word] = 1;
        }
        for (word = WORD_LIST; *word; word++) {
            strcpy(buf, *word);
            res = dict[buf];
        }
    }
}

# ifdef HAVE_SPARSE_HASH
using google::dense_hash_map;

static void google_dense_hash()
{
    for (int i = 0; i < N; i++) {
        dense_hash_map<const char*, int, __gnu_cxx::hash<const char*>, eqstr> dict;
        dict.set_empty_key(NULL);
        const char **word;
        char buf[100];
        long res;
        for (word = WORD_LIST; *word; word++) {
            strcpy(buf, *word);
            dict[*word] = 1;
            res = dict[buf];
        }
    }
}

using google::sparse_hash_map;

static void google_sparse_hash()
{
    for (int i = 0; i < N; i++) {
        sparse_hash_map<const char*, int, __gnu_cxx::hash<const char*>, eqstr> dict;
        const char **word;
        char buf[100];
        long res;
        for (word = WORD_LIST; *word; word++) {
            strcpy(buf, *word);
            dict[*word] = 1;
            res = dict[buf];
        }
    }
}
# endif

//using std::hash_map;
//
//static void stlport_hash()
//{
//    for (int i = 0; i < N; i++) {
//        hash_map<const char*, int, std::hash<const char*>, eqstr> dict;
//        const char **word;
//        char buf[100];
//        long res;
//        for (word = WORD_LIST; *word; word++) {
//            strcpy(buf, *word);
//            dict[*word] = 1;
//            res = dict[buf];
//        }
//    }
//}

#endif

BENCH(hash_implementations)
{
    BM_ADD(ferret_hash);
#ifdef __cplusplus
    BM_ADD(stdcpp_hash);
# ifdef HAVE_SPARSE_HASH
    BM_ADD(google_dense_hash);
    BM_ADD(google_sparse_hash);
# endif
    //BM_ADD(stlport_hash);
#endif
}

static void standard_hash()
{
    int i;
    for (i = 0; i < N; i++) {
        Hash *h = h_new_str(NULL, NULL);
        const char **word;
        char buf[100];
        long res;
        for (word = WORD_LIST; *word; word++) {
            h_set(h, *word, (void *)1);
            strcpy(buf, *word);
            res = (long)h_get(h, buf);
        }
        h_destroy(h);
    }
}

#define PERTURB_SHIFT 5
static char *dummy_key = "";
static HashEntry *h_lookup_str(Hash *ht, register const void *key)
{
    register const unsigned long hash = str_hash((const char *)key);
    register unsigned int perturb;
    register int mask = ht->mask;
    register HashEntry *he0 = ht->table;
    register int i = hash & mask;
    register HashEntry *he = &he0[i];
    register HashEntry *freeslot = NULL;

    if (he->key == NULL || he->key == key) {
        he->hash = hash;
        return he;
    }
    if (he->key == dummy_key) {
        freeslot = he;
    }
    else {
        if ((he->hash == hash)
            && 0 == strcmp((const char *)he->key, (const char *)key)) {
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
                && he->key != dummy_key
                && 0 == strcmp((const char *)he->key, (const char *)key))) {
            return he;
        }
        if (he->key == dummy_key && freeslot == NULL) {
            freeslot = he;
        }
    }
}

static void string_hash()
{
    int i;
    for (i = 0; i < N; i++) {
        Hash *h = h_new_str(NULL, NULL);
        const char **word;
        char buf[100];
        long res;
        h->lookup_i = &h_lookup_str;
        for (word = WORD_LIST; *word; word++) {
            h_set(h, *word, (void *)1);
            strcpy(buf, *word);
            res = (long)h_get(h, buf);
        }
        h_destroy(h);
    }
}

BENCH(specialized_string_hash)
{
    BM_ADD(standard_hash);
    BM_ADD(string_hash);
}
