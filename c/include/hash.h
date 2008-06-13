#ifndef FRT_HASH_H
#define FRT_HASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "global.h"

/****************************************************************************
 *
 * Hash
 *
 ****************************************************************************/

#define FRT_HASH_MINSIZE 8
#define FRT_SLOW_DOWN 50000     /* stop increasing the hash table so quickly to
                                 * conserve memory */

/**
 * Return values for h_set
 */
typedef enum
{
    FRT_HASH_KEY_DOES_NOT_EXIST = 0,
    FRT_HASH_KEY_EQUAL = 1,
    FRT_HASH_KEY_SAME = 2
} FrtHashKeyStatus;

/**
 * struct used internally to store values in the Hash
 */
typedef struct
{
    unsigned long hash;
    void *key;
    void *value;
} FrtHashEntry;

/**
 * As the hash table is filled and entries are deleted, Dummy HashEntries are
 * put in place. We therefor keep two counts. +size+ is the number active
 * elements and +fill+ is the number of active elements together with the
 * number of dummy elements. +fill+ is basically just kept around so that we
 * know when to resize. The Hash is resized when more than two thirds of
 * the Hash is Filled.
 */
typedef struct FrtHash
{
    int fill;                   /* num Active + num Dummy */
    int size;                   /* num Active ie, num keys set */
    int mask;                   /* capacity_of_table - 1 */
    int ref_cnt;

    /* table points to smalltable initially. If the table grows beyond 2/3 of
     * HASH_MINSIZE it will point to newly malloced memory as it grows. */
    FrtHashEntry *table;

    /* When a Hash is created it needs an initial table to start if off.
     * All Hashs will start with smalltable and then malloc a larger
     * table as the Hash grows */
    FrtHashEntry smalltable[FRT_HASH_MINSIZE];

    /* the following function pointers are used internally and should not be
     * used outside of the Hash methods */
    FrtHashEntry *(*lookup_i)(struct FrtHash *self,
                              register const void *key);
    unsigned long (*hash_i)(const void *key);
    int           (*eq_i)(const void *key1, const void *key2);
    void          (*free_key_i)(void *p);
    void          (*free_value_i)(void *p);
} FrtHash;

/**
 * Hashing function type used by Hash. A function of this type must be
 * passed to create a new Hash.
 *
 * @param key object to hash
 * @return an unsigned 32-bit integer hash value
 */
typedef unsigned long (*frt_hash_ft)(const void *key);

/**
 * Equals function type used by Hash. A function of this type must be
 * passed to create a new Hash.
 */
typedef int (*frt_eq_ft)(const void *key1, const void *key2);

/**
 * Determine a hash value for a string. The string must be null terminated
 *
 * @param str string to hash
 * @return an unsigned long integer hash value
 */
extern unsigned long frt_str_hash(const char *const str);

/**
 * Determine a hash value for a pointer. Just cast the pointer to an unsigned
 * long.
 *
 * @param ptr pointer to hash
 * @return an unsigned long integer hash value
 */
extern unsigned long frt_ptr_hash(const void *const ptr);

/**
 * Determine if two pointers point to the same point in memory.
 *
 * @param q1 first pointer
 * @param q2 second pointer
 * @return true if the pointers are equal
 */
extern int frt_ptr_eq(const void *q1, const void *q2);

/**
 * Create a new Hash that uses any type of object as its key. The
 * Hash will store all keys and values so if you want to destroy those
 * values when the Hash is destroyed then you should pass free functions.
 * NULL will suffice otherwise.
 *
 * @param hash function to determine the hash value of a key in the Hash
 * @param eq function to determine the equality of to keys in the Hash
 * @param free_key function to free the key stored in the Hash when an
 *    entry is deleted, replaced or when the Hash is destroyed. If you
 *    pass NULL in place of this parameter the key will not be destroyed.
 * @param free_value function to free the value stored in the Hash when
 *    an entry is deleted, replaced or when the Hash is destroyed. If you
 *    pass NULL in place of this parameter the value will not be destroyed.
 * @return A newly allocated Hash
 */
extern FrtHash *frt_h_new(frt_hash_ft hash,
                          frt_eq_ft eq,
                          frt_free_ft free_key,
                          frt_free_ft free_value);

/**
 * Create a new Hash that uses null-terminated strings as its keys. The
 * Hash will store all keys and values so if you want to destroy those
 * values when the Hash is destroyed then you should pass free functions.
 * NULL will suffice otherwise.
 *
 * @param free_key function to free the key stored in the Hash when an
 *    entry is deleted, replaced or when the Hash is destroyed. If you
 *    pass NULL in place of this parameter the key will not be destroyed.
 * @param free_value function to free the value stored in the Hash when
 *    an entry is deleted, replaced or when the Hash is destroyed. If you
 *    pass NULL in place of this parameter the value will not be destroyed.
 * @return A newly allocated Hash
 */
extern FrtHash *frt_h_new_str(frt_free_ft free_key,
                              frt_free_ft free_value);

/**
 * Create a new Hash that uses integers as its keys. The Hash will store all
 * values so if you want to destroy those values when the Hash is destroyed
 * then you should pass a free function.  NULL will suffice otherwise.
 *
 * @param free_value function to free the value stored in the Hash when
 *    an entry is deleted, replaced or when the Hash is destroyed. If you
 *    pass NULL in place of this parameter the value will not be destroyed.
 * @return A newly allocated Hash
 */
extern FrtHash *frt_h_new_int(frt_free_ft free_value);

/**
 * Create a new Hash that uses pointers as its keys. The Hash will store all
 * values so if you want to destroy those values when the Hash is destroyed
 * then you should pass a free function. NULL will suffice otherwise.
 *
 * @param free_value function to free the value stored in the Hash when
 *    an entry is deleted, replaced or when the Hash is destroyed. If you
 *    pass NULL in place of this parameter the value will not be destroyed.
 * @return A newly allocated Hash
 */
#define frt_h_new_ptr(free_value) frt_h_new_int(free_value)

/**
 * Destroy the Hash. This function will also destroy all keys and values
 * in the Hash depending on how the free_key and free_value were set.
 *
 * @param self the Hash to destroy
 */
extern void frt_h_destroy(FrtHash *self);

/**
 * Clear the Hash. This function will delete all keys and values from the
 * hash table, also destroying all keys and values in the Hash depending
 * on how the free_key and free_value were set.
 *
 * @param self the Hash to clear
 */
extern void frt_h_clear(FrtHash *self);

/**
 * Get the value in the Hash referenced by the key +key+.
 *
 * @param self the Hash to reference
 * @param key the key to lookup
 * @return the value referenced by the key +key+. If there is no value
 *   referenced by that key, NULL is returned.
 */
extern void *frt_h_get(FrtHash *self, const void *key);

/**
 * Delete the value in Hash referenced by the key +key+. When the value
 * is deleted it is also destroyed along with the key depending on how
 * free_key and free_value where set when the Hash was created. If you
 * don't want to destroy the value use h_rem.
 *
 * This functions returns +true+ if the value was deleted successfully or
 * false if the key was not found.
 *
 * @see h_rem
 *
 * @param self the Hash to reference
 * @param key the key to lookup
 * @return true if the object was successfully deleted or false if the key was
 *   not found
 */
extern int frt_h_del(FrtHash *self, const void *key);

/**
 * Remove the value in Hash referenced by the key +key+. When the value
 * is removed it is returned rather than destroyed. The key however is
 * destroyed using the free_key functions passed when the Hash is created
 * if del_key is true.
 *
 * If you want the value to be destroyed, use the h_del function.
 *
 * @see h_del
 *
 * @param self the Hash to reference
 * @param key the key to lookup
 * @param del_key set to true if you want the key to be deleted when the value
 *   is removed from the Hash
 * @return the value referenced by +key+ if it can be found or NULL otherwise
 */
extern void *frt_h_rem(FrtHash *self, const void *key, bool del_key);

/**
 * WARNING: this function may destroy an old value or key if the key already
 * exists in the Hash, depending on how free_value and free_key were set
 * for this Hash.
 *
 * Add the value +value+ to the Hash referencing it with key +key+.
 *
 * When a value is added to the Hash it replaces any value that
 * might already be stored under that key. If free_value is already set then
 * the old value will be freed using that function.
 *
 * Similarly the old key might replace be replaced by the new key if they are
 * are equal (according to the Hash's eq function) but seperately
 * allocated objects.
 *
 * @param self the Hash to add the value to
 * @param key the key to use to reference the value
 * @param value the value to add to the Hash
 * @return one of three values;
 *   <pre>
 *     HASH_KEY_DOES_NOT_EXIST  there was no value stored with that key
 *     HASH_KEY_EQUAL           the key existed and was seperately allocated.
 *                              In this situation the old key will have been
 *                              destroyed if free_key was set
 *     HASH_KEY_SAME            the key was identical (same memory pointer) to
 *                              the existing key so no key was freed
 *   </pre>
 */
extern FrtHashKeyStatus frt_h_set(FrtHash *self,
                                     const void *key, void *value);

/**
 * Add the value +value+ to the Hash referencing it with key +key+. If
 * the key already exists in the Hash, the value won't be added and the
 * function will return false. Otherwise it will return true.
 *
 * @param self the Hash to add the value to
 * @param key the key to use to reference the value
 * @param value the value to add to the Hash
 * @return true if the value was successfully added or false otherwise
 */
extern int frt_h_set_safe(FrtHash *self, const void *key, void *value);

/**
 * Return a hash entry object so you can handle the insert yourself. This can
 * be used for performance reasons or for more control over how a value is
 * added. Say, for example, you wanted to append a value to an array, or add a
 * new array if non-existed, you could use this method by checking the value
 * of the HashEntry returned.
 *
 * @param self the Hash to add the value to
 * @param key the key to use to reference the value
 * @param he HashEntry a pointer to the hash entry object now reserved for this
 * value. Be sure to set both the *key* and the *value*
 * @return true if the key was empty, false otherwise
 */
extern FRT_INLINE bool frt_h_set_ext(FrtHash *self,
                                     const void *key,
                                     FrtHashEntry **he);

/**
 * Check whether key +key+ exists in the Hash.
 *
 * @param self the Hash to check in
 * @param key the key to check for in the Hash
 * @return one of three values;
 *   <pre>
 *     0 - HASH_KEY_DOES_NOT_EXIST  there was no value stored with that key
 *     1 - HASH_KEY_EQUAL           the key existed and was seperately
 *                                  allocated.
 *     2 - HASH_KEY_SAME            the key was identical (same memory
 *                                  pointer) to the existing key so no key was
 *                                  freed
 *   </pre>
 *   Note: the return value can be treated as a true/false value, ie 0 if the
 *   key doesn't exist, non-zero if it does.
 */
extern FrtHashKeyStatus frt_h_has_key(FrtHash *self,
                                         const void *key);

/**
 * Get the value in the Hash referenced by an integer key +key+.
 *
 * @param self the Hash to reference
 * @param key the integer key to lookup
 * @return the value referenced by the key +key+. If there is no value
 *   referenced by that key, NULL is returned.
 */
extern void *frt_h_get_int(FrtHash *self, const unsigned long key);

/**
 * Delete the value in Hash referenced by the integer key +key+. When the
 * value is deleted it is also destroyed using the free_value function. If you
 * don't want to destroy the value use h_rem.
 *
 * This functions returns +true+ if the value was deleted successfully or
 * false if the key was not found.
 *
 * @see h_rem
 *
 * @param self the Hash to reference
 * @param key the integer key to lookup
 * @return true if the object was successfully deleted or false if the key was
 *   not found
 */
extern int frt_h_del_int(FrtHash *self, const unsigned long key);

/**
 * Remove the value in Hash referenced by the integer key +key+. When the
 * value is removed it is returned rather than destroyed.
 *
 * If you want the value to be destroyed, use the h_del function.
 *
 * @see h_del
 *
 * @param self the Hash to reference
 * @param key the integer key to lookup
 * @return the value referenced by +key+ if it can be found or NULL otherwise
 */
extern void *frt_h_rem_int(FrtHash *self, const unsigned long key);

/**
 * WARNING: this function may destroy an old value if the key already exists
 * in the Hash, depending on how free_value was set for this Hash.
 *
 * Add the value +value+ to the Hash referencing it with an integer key
 * +key+.
 *
 * When a value is added to the Hash it replaces any value that
 * might already be stored under that key. If free_value is already set then
 * the old value will be freed using that function.
 *
 * Similarly the old key might replace be replaced by the new key if they are
 * are equal (according to the Hash's eq function) but seperately
 * allocated objects.
 *
 * @param self the Hash to add the value to
 * @param key the integer key to use to reference the value
 * @param value the value to add to the Hash
 * @return one of three values;
 *   <pre>
 *     HASH_KEY_DOES_NOT_EXIST  there was no value stored with that key
 *     HASH_KEY_EQUAL           the key existed and was seperately allocated.
 *                              In this situation the old key will have been
 *                              destroyed if free_key was set
 *     HASH_KEY_SAME            the key was identical (same memory pointer) to
 *                              the existing key so no key was freed
 *   </pre>
 */
extern FrtHashKeyStatus frt_h_set_int(FrtHash *self,
                                         const unsigned long key,
                                         void *value);

/**
 * Add the value +value+ to the Hash referencing it with integer key
 * +key+. If the key already exists in the Hash, the value won't be added
 * and the function will return false. Otherwise it will return true.
 *
 * @param self the Hash to add the value to
 * @param key the integer key to use to reference the value
 * @param value the value to add to the Hash
 * @return true if the value was successfully added or false otherwise
 */
extern int frt_h_set_safe_int(FrtHash *self,
                              const unsigned long key,
                              void *value);
/**
 * Check whether integer key +key+ exists in the Hash.
 *
 * @param self the Hash to check in
 * @param key the integer key to check for in the Hash
 * @return true if the key exists in the Hash, false otherwise.
 */
extern int frt_h_has_key_int(FrtHash *self, const unsigned long key);

typedef void (*frt_h_each_key_val_ft)(void *key, void *value, void *arg);

/**
 * Run function +each_key_val+ on each key and value in the Hash. The third
 * argument +arg+ will also be passed to +each_key_val+. If you need to pass
 * more than one argument to the function you should pass a struct.
 *
 * example;
 *
 * // Lets say we have stored strings in a Hash and we want to put them
 * // all into an array. First we need to create a struct to store the
 * // strings;
 *
 * struct StringArray {
 *   char **strings;
 *   int cnt;
 *   int size;
 * };
 *
 * static void add_string_ekv(void *key, void *value,
 *                            struct StringArray *str_arr)
 * {
 *   str_arr->strings[str_arr->cnt] = (char *)value;
 *   str_arr->cnt++;
 * }
 *
 * struct StringArray *h_extract_strings(Hash *ht)
 * {
 *   struct StringArray *str_arr = FRT_ALLOC(struct StringArray);
 *
 *   str_arr->strings = FRT_ALLOC_N(char *, ht->size);
 *   str_arr->cnt = 0;
 *   str_arr->size = ht->size;
 *
 *   h_each(ht, (h_each_key_val_ft)add_string_ekv, str_arr);
 *
 *   return str_arr;
 * }
 *
 * @param self the Hash to run the function on
 * @param each_key_val function to run on on each key and value in the
 *   Hash
 * @param arg an extra argument to pass to each_key_val each time it is called
 */
extern void frt_h_each(FrtHash *self,
                       void (*each_key_val)(void *key, void *value, void *arg),
                       void *arg);

typedef void *(*frt_h_clone_ft)(void *val);
/**
 * Clone the Hash as well as cloning each of the keys and values if you
 * want to do a deep clone. To do a deep clone you will need to pass a
 * clone_key function and/or a clone_value function.
 *
 * @param self the Hash to clone
 * @param clone_key the function to clone the key with
 * @param clone_value the function to clone the value with
 * @return a clone of the original Hash
 */
extern FrtHash *frt_h_clone(FrtHash *self,
                                    frt_h_clone_ft clone_key,
                                    frt_h_clone_ft clone_value);

/*
 * The following functions should only be used in static Hash
 * declarations
 */
/**
 * This is the lookup function for a hash table with non-string keys. The
 * hash() and eq() methods used are stored in the hash table. This method will
 * always return a HashEntry. If there is no entry with the given key then an
 * empty entry will be returned with the key set to the key that was passed.
 *
 * @param ht the hash table to look in
 * @param key the key to lookup
 * @return the HashEntry that was found
 */
extern FrtHashEntry *frt_h_lookup(FrtHash *ht,
                                     register const void *key);

typedef FrtHashEntry *(*frt_h_lookup_ft)(FrtHash *ht,
                                            register const void *key);

extern void frt_h_str_print_keys(FrtHash *ht, FILE *out);

/**
 * The Hash implementation actually keeps a buffer of old hash tables around
 * for performance reasons. If you want all memory freed when your program
 * finishes (useful if you are using valgrind) you should call this method on
 * exit.
 *
 * One way to do this is to register it with atexit(). This is done for you
 * when you call +frt_init+.
 */
extern void frt_hash_finalize();

#ifdef __cplusplus
} // extern "C"
#endif

#endif
