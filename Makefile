
CFLAGS = -std=c99 -pedantic -Wall -Wextra -Iinclude -Ilib/libstemmer_c/include -fno-common -g -DDEBUG #-D_FILE_OFFSET_BITS=64

LFLAGS = -lm -lpthread

include lib/libstemmer_c/mkinc.mak

STEMMER_OBJS = $(patsubst %.c,src/libstemmer_c/%.o, $(snowball_sources))

TEST_OBJS = test_priorityqueue.o test_hashset.o test_helper.o test_test.o test.o test_global.o test_bitvector.o test_hash.o test_ram_store.o test_store.o test_term_vectors.o test_fs_store.o test_except.o test_document.o test_fields.o test_segments.o test_analysis.o test_term.o test_array.o test_similarity.o test_mem_pool.o testhelper.o test_index.o test_search.o test_compound_io.o test_q_filtered.o test_q_fuzzy.o test_filter.o test_q_span.o test_q_const_score.o test_sort.o

OBJS = priorityqueue.o hashset.o helper.o global.o bitvector.o hash.o fs_store.o posh.o except.o ram_store.o store.o analysis.o document.o search.o similarity.o stopwords.o array.o index.o mem_pool.o compound_io.o q_prefix.o q_range.o q_phrase.o q_term.o sort.o q_boolean.o filter.o q_const_score.o q_match_all.o q_wildcard.o q_fuzzy.o q_multi_term.o q_filtered_query.o q_span.o libstemmer.o

vpath %.c test src

vpath %.h test include lib/libstemmer_c/include

runtests: testall
	./testall -v -f -q

testall: $(OBJS) $(TEST_OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) $(OBJS) $(TEST_OBJS) -o testall

valgrind: testall
	valgrind --leak-check=yes -v testall -q

libstemmer.o: $(snowball_sources:%.c=lib/libstemmer_c/%.o)
	$(AR) -cru $@ $^

.PHONY: clean
clean:
	rm -f *.o testall gmon.out

testhelper.o: defines.h except.h posh.h testhelper.h global.h

except.o: defines.h threading.h except.h posh.h global.h

priorityqueue.o: defines.h priorityqueue.h except.h posh.h global.h

q_term.o: threading.h defines.h analysis.h mem_pool.h priorityqueue.h index.h except.h hash.h posh.h search.h similarity.h global.h hashset.h store.h bitvector.h document.h

q_phrase.o: threading.h defines.h analysis.h mem_pool.h priorityqueue.h index.h except.h hash.h posh.h search.h similarity.h global.h hashset.h array.h store.h bitvector.h document.h

test_hash.o: defines.h except.h hash.h posh.h test.h global.h

q_multi_term.o: threading.h analysis.h mem_pool.h defines.h priorityqueue.h index.h hash.h except.h posh.h search.h similarity.h hashset.h global.h helper.h store.h bitvector.h document.h

similarity.o: threading.h defines.h analysis.h mem_pool.h priorityqueue.h index.h except.h hash.h posh.h search.h global.h hashset.h similarity.h array.h helper.h store.h bitvector.h document.h

q_prefix.o: threading.h defines.h analysis.h mem_pool.h priorityqueue.h index.h except.h hash.h posh.h search.h similarity.h global.h hashset.h store.h bitvector.h document.h

test_term_vectors.o: priorityqueue.h mem_pool.h analysis.h defines.h threading.h hash.h except.h index.h posh.h test.h similarity.h hashset.h global.h store.h bitvector.h document.h

test_similarity.o: defines.h except.h posh.h test.h global.h similarity.h

bitvector.o: defines.h except.h posh.h global.h bitvector.h

test_q_const_score.o: threading.h analysis.h mem_pool.h priorityqueue.h defines.h index.h hash.h except.h posh.h test.h search.h similarity.h hashset.h global.h store.h bitvector.h document.h

test_q_filtered.o: threading.h analysis.h mem_pool.h priorityqueue.h defines.h index.h hash.h except.h posh.h test.h search.h similarity.h hashset.h global.h store.h bitvector.h document.h

test_document.o: defines.h hash.h except.h posh.h test.h global.h document.h

test_filter.o: threading.h analysis.h mem_pool.h priorityqueue.h defines.h index.h hash.h except.h posh.h testhelper.h test.h search.h similarity.h hashset.h global.h store.h bitvector.h document.h

store.o: threading.h defines.h hash.h except.h posh.h global.h store.h

ram_store.o: threading.h defines.h hash.h except.h posh.h global.h store.h

test_search.o: threading.h analysis.h mem_pool.h priorityqueue.h defines.h index.h hash.h except.h posh.h testhelper.h test.h search.h similarity.h hashset.h global.h array.h helper.h store.h bitvector.h document.h

q_fuzzy.o: threading.h defines.h analysis.h mem_pool.h priorityqueue.h index.h except.h hash.h posh.h search.h similarity.h global.h hashset.h helper.h store.h bitvector.h document.h

hash.o: defines.h except.h hash.h posh.h global.h

compound_io.o: priorityqueue.h mem_pool.h analysis.h defines.h threading.h hash.h except.h index.h posh.h similarity.h hashset.h global.h array.h store.h bitvector.h document.h

test_q_span.o: threading.h analysis.h mem_pool.h priorityqueue.h defines.h index.h hash.h except.h posh.h test.h search.h similarity.h hashset.h global.h store.h bitvector.h document.h

test_compound_io.o: priorityqueue.h mem_pool.h analysis.h defines.h threading.h hash.h except.h index.h posh.h testhelper.h test.h similarity.h hashset.h global.h store.h bitvector.h document.h

test_hashset.o: defines.h hash.h except.h posh.h test.h global.h hashset.h

filter.o: threading.h defines.h analysis.h mem_pool.h priorityqueue.h index.h except.h hash.h posh.h search.h similarity.h global.h hashset.h store.h bitvector.h document.h

search.o: threading.h defines.h analysis.h mem_pool.h priorityqueue.h index.h except.h hash.h posh.h search.h similarity.h global.h hashset.h array.h store.h bitvector.h document.h

q_const_score.o: threading.h defines.h analysis.h mem_pool.h priorityqueue.h index.h except.h hash.h posh.h search.h similarity.h global.h hashset.h store.h bitvector.h document.h

q_span.o: threading.h defines.h analysis.h mem_pool.h priorityqueue.h index.h except.h hash.h posh.h search.h similarity.h global.h hashset.h store.h bitvector.h document.h

mem_pool.o: defines.h mem_pool.h except.h posh.h global.h

test_test.o: defines.h except.h posh.h test.h global.h

test_sort.o: threading.h analysis.h mem_pool.h priorityqueue.h defines.h index.h hash.h except.h posh.h test.h search.h similarity.h hashset.h global.h store.h bitvector.h document.h

posh.o: posh.h

test_fields.o: priorityqueue.h mem_pool.h analysis.h defines.h threading.h hash.h except.h index.h posh.h test.h similarity.h hashset.h global.h store.h bitvector.h document.h

test_bitvector.o: defines.h except.h posh.h test.h global.h bitvector.h

test_store.o: threading.h defines.h hash.h except.h posh.h test.h global.h store.h

fs_store.o: threading.h defines.h hash.h except.h posh.h global.h store.h

index.o: mem_pool.h analysis.h threading.h defines.h priorityqueue.h hash.h except.h index.h posh.h hashset.h global.h similarity.h helper.h array.h store.h bitvector.h document.h

q_range.o: threading.h defines.h analysis.h mem_pool.h priorityqueue.h index.h except.h hash.h posh.h search.h similarity.h global.h hashset.h store.h bitvector.h document.h

sort.o: priorityqueue.h mem_pool.h analysis.h defines.h threading.h hash.h except.h index.h posh.h search.h similarity.h hashset.h global.h store.h bitvector.h document.h

q_match_all.o: threading.h defines.h analysis.h mem_pool.h priorityqueue.h index.h except.h hash.h posh.h search.h similarity.h global.h hashset.h store.h bitvector.h document.h

test_fs_store.o: threading.h defines.h hash.h except.h posh.h test.h global.h test_store.h store.h

test_segments.o: priorityqueue.h mem_pool.h analysis.h defines.h threading.h hash.h except.h index.h posh.h test.h similarity.h hashset.h global.h store.h bitvector.h document.h

test_ram_store.o: threading.h defines.h hash.h except.h posh.h test.h global.h test_store.h store.h

test.o: defines.h all_tests.h except.h posh.h test.h global.h

test_term.o: priorityqueue.h mem_pool.h analysis.h defines.h threading.h hash.h except.h index.h posh.h test.h similarity.h hashset.h global.h store.h bitvector.h document.h

test_global.o: defines.h except.h posh.h test.h global.h

q_boolean.o: threading.h defines.h analysis.h mem_pool.h priorityqueue.h index.h except.h hash.h posh.h search.h similarity.h global.h hashset.h array.h store.h bitvector.h document.h

array.o: defines.h except.h posh.h global.h array.h

global.o: defines.h except.h posh.h global.h

analysis.o: defines.h analysis.h except.h hash.h posh.h global.h

test_index.o: priorityqueue.h mem_pool.h analysis.h defines.h threading.h hash.h except.h index.h posh.h testhelper.h test.h similarity.h hashset.h global.h store.h bitvector.h document.h

test_priorityqueue.o: defines.h priorityqueue.h except.h posh.h test.h global.h

document.o: defines.h hash.h except.h posh.h global.h document.h

test_helper.o: defines.h except.h posh.h test.h global.h helper.h

test_q_fuzzy.o: threading.h analysis.h mem_pool.h priorityqueue.h defines.h index.h hash.h except.h posh.h test.h search.h similarity.h hashset.h global.h store.h bitvector.h document.h

test_analysis.o: defines.h analysis.h hash.h except.h posh.h test.h global.h

q_filtered_query.o: threading.h defines.h analysis.h mem_pool.h priorityqueue.h index.h except.h hash.h posh.h search.h similarity.h global.h hashset.h store.h bitvector.h document.h

test_except.o: defines.h except.h posh.h test.h global.h

helper.o: defines.h posh.h helper.h

test_mem_pool.o: defines.h mem_pool.h except.h posh.h test.h global.h

test_array.o: defines.h except.h posh.h test.h global.h array.h

hashset.o: defines.h hash.h except.h posh.h global.h hashset.h

q_wildcard.o: threading.h defines.h analysis.h mem_pool.h priorityqueue.h index.h except.h hash.h posh.h search.h similarity.h global.h hashset.h store.h bitvector.h document.h


