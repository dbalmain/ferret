#ifndef FRT_MAPPER_H
#define FRT_MAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hash.h"

typedef struct FrtState
{
    int  (*next)(struct FrtState *self, int c, int *states);
    void (*destroy_i)(struct FrtState *self);
    int  (*is_match)(struct FrtState *self, char **mapping);
} FrtState;

typedef struct FrtDeterministicState
{
    struct FrtDeterministicState *next[256];
    int longest_match;
    char *mapping;
    int mapping_len;
} FrtDeterministicState;

typedef struct FrtMapping
{
    char *pattern;
    char *replacement;
} FrtMapping;

typedef struct FrtMultiMapper
{
    FrtMapping **mappings;
    int size;
    int capa;
    FrtDeterministicState **dstates;
    int d_size;
    int d_capa;
    unsigned char alphabet[256];
    int a_size;
    FrtHash *dstates_map;
    FrtState **nstates;
    int nsize;
    int *next_states;
    int ref_cnt;
} FrtMultiMapper;

extern FrtMultiMapper *frt_mulmap_new();
extern void frt_mulmap_add_mapping(FrtMultiMapper *self, const char *p, const char *r);
extern void frt_mulmap_compile(FrtMultiMapper *self);
extern char *frt_mulmap_map(FrtMultiMapper *self, char *to, char *from, int capa);
extern char *frt_mulmap_dynamic_map(FrtMultiMapper *self, char *from);
extern int frt_mulmap_map_len(FrtMultiMapper *self, char *to, char *from, int capa);
extern void frt_mulmap_destroy(FrtMultiMapper *self);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
