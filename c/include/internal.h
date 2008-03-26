#ifndef FRT_INTERNAL_H
#define FRT_INTERNAL_H

/* Constants */
#define EXIT         FRT_EXIT
#define HASH_MINSIZE FRT_HASH_MINSIZE
#define SLOW_DOWN    FRT_SLOW_DOWN

/* Types */
#define HashEntry     FerretHashEntry
#define HashKeyStatus FerretHashKeyStatus
#define HashTable     FerretHashTable
#define PriorityQueue FerretPriorityQueue

/* Functions */
#define calloc            frt_calloc
#define clean_up          frt_clean_up
#define eq_ft             frt_eq_ft
#define free_ft           frt_free_ft
#define h_clear           frt_h_clear
#define h_clone           frt_h_clone
#define h_clone_func_t    frt_h_clone_func_t
#define h_del             frt_h_del
#define h_del_int         frt_h_del_int
#define h_destroy         frt_h_destroy
#define h_each            frt_h_each
#define h_each_key_val_ft frt_h_each_key_val_ft
#define h_get             frt_h_get
#define h_get_int         frt_h_get_int
#define h_has_key         frt_h_has_key
#define h_has_key_int     frt_h_has_key_int
#define h_lookup          frt_h_lookup
#define h_lookup_ft       frt_h_lookup_ft
#define h_new             frt_h_new
#define h_new_int         frt_h_new_int
#define h_new_str         frt_h_new_str
#define h_rem             frt_h_rem
#define h_rem_int         frt_h_rem_int
#define h_set             frt_h_set
#define h_set_ext         frt_h_set_ext
#define h_set_int         frt_h_set_int
#define h_set_safe        frt_h_set_safe
#define h_set_safe_int    frt_h_set_safe_int
#define h_str_print_keys  frt_h_str_print_keys
#define hash_ft           frt_hash_ft
#define init              frt_init
#define lt_ft             frt_lt_ft
#define micro_sleep       frt_micro_sleep
#define pq_clear          frt_pq_clear
#define pq_clone          frt_pq_clone
#define pq_destroy        frt_pq_destroy
#define pq_down           frt_pq_down
#define pq_free           frt_pq_free
#define pq_full           frt_pq_full
#define pq_insert         frt_pq_insert
#define pq_new            frt_pq_new
#define pq_pop            frt_pq_pop
#define pq_push           frt_pq_push
#define pq_top            frt_pq_top
#define progname          frt_progname
#define ptr_eq            frt_ptr_eq
#define ptr_hash          frt_ptr_hash
#define setprogname       frt_setprogname
#define str_hash          frt_str_hash

#endif
