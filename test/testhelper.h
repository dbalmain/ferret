#ifndef FRT_T_HELPER_H
#define FRT_T_HELPER_H

#define TEST_WORD_LIST_SIZE 1930
#define TEST_WORD_LIST_MAX_LEN 23

extern const char *test_word_list[];

extern char *make_random_string(char *buf, int num_words);
extern char *get_nth_word(char *words, char *buf, int n, int *s, int *e);
extern int nth_word_eql(char *words, char *word, int n);

#endif
