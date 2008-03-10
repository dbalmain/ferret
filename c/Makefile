CFLAGS = -std=c99 -pedantic -Wall -Wextra -fno-stack-protector -Iinclude -Ilib/libstemmer_c/include -fno-common -D_FILE_OFFSET_BITS=64 -g -DDEBUG 

LFLAGS = -lm -lpthread

include lib/libstemmer_c/mkinc.mak

STEMMER_OBJS = $(patsubst %.c,src/libstemmer_c/%.o, $(snowball_sources))

TEST_OBJS = test_multimapper.o test_q_const_score.o test_threading.o test_q_fuzzy.o test_term.o test_q_filtered.o testhelper.o test_hash.o test_fs_store.o test_segments.o test_similarity.o test_except.o test_store.o test_highlighter.o test_hashset.o test_global.o test_bitvector.o test_search.o test_fields.o test_index.o test_mempool.o test_compound_io.o test_q_parser.o test_term_vectors.o test_array.o test_document.o test_helper.o test_test.o test.o test_q_span.o test_analysis.o test_filter.o test_priorityqueue.o test_sort.o test_ram_store.o test_file_deleter.o

OBJS = q_multi_term.o hashset.o q_match_all.o bitvector.o mempool.o index.o compound_io.o q_wildcard.o ind.o priorityqueue.o q_range.o q_prefix.o array.o hash.o fs_store.o multimapper.o similarity.o except.o ram_store.o q_boolean.o q_phrase.o global.o q_filtered_query.o q_const_score.o term_vectors.o stopwords.o search.o q_fuzzy.o posh.o q_parser.o document.o helper.o q_span.o analysis.o filter.o q_term.o sort.o store.o libstemmer.o

vpath %.c test src

vpath %.h test include lib/libstemmer_c/include

runtests: testall
	./testall -v -f -q

testall: $(OBJS) $(TEST_OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) $(OBJS) $(TEST_OBJS) -o testall

valgrind: testall
	valgrind --leak-check=yes --show-reachable=yes --workaround-gcc296-bugs=yes -v ./testall -q

b: load_bench
	./load_bench

load_bench: load_bench.c $(OBJS)
	$(CC) $(CFLAGS) -lpthread load_bench.c $(OBJS) $(LFLAGS) -o load_bench

bench: bench.c $(OBJS)
	$(CC) $(CFLAGS) -lpthread bench.c $(OBJS) $(LFLAGS) -o bench

search_bench: search_bench.c $(OBJS)
	$(CC) $(CFLAGS) search_bench.c $(OBJS) $(LFLAGS) -o search_bench

sort_bench: sort_bench.c $(OBJS)
	$(CC) $(CFLAGS) sort_bench.c $(OBJS) $(LFLAGS) -o sort_bench

libstemmer.o: $(snowball_sources:%.c=lib/libstemmer_c/%.o)
	$(AR) -cru $@ $^

q_parser.o: src/q_parser.c
	$(CC) $(CFLAGS) src/q_parser.c -c

src/q_parser.c: src/q_parser.y

.PHONY: clean
clean:
	rm -f *.o testall gmon.out bench search_bench
testhelper.o: except.h posh.h testhelper.h global.h lang.h config.h

except.o: threading.h except.h posh.h global.h lang.h config.h

priorityqueue.o: priorityqueue.h except.h posh.h global.h lang.h config.h

q_term.o: threading.h analysis.h priorityqueue.h index.h except.h hash.h multimapper.h posh.h mempool.h search.h similarity.h global.h hashset.h lang.h store.h bitvector.h config.h document.h

q_phrase.o: threading.h analysis.h priorityqueue.h index.h except.h hash.h multimapper.h posh.h mempool.h search.h similarity.h global.h hashset.h lang.h array.h store.h bitvector.h config.h document.h

test_hash.o: except.h hash.h posh.h test.h global.h lang.h config.h

q_multi_term.o: threading.h analysis.h priorityqueue.h index.h hash.h except.h multimapper.h posh.h mempool.h search.h similarity.h hashset.h global.h lang.h helper.h store.h bitvector.h document.h config.h

similarity.o: threading.h analysis.h priorityqueue.h index.h except.h hash.h multimapper.h posh.h mempool.h search.h global.h hashset.h similarity.h lang.h array.h helper.h store.h bitvector.h config.h document.h

test_term_vectors.o: priorityqueue.h analysis.h threading.h hash.h except.h index.h multimapper.h posh.h mempool.h test.h similarity.h hashset.h global.h lang.h store.h bitvector.h document.h config.h

q_prefix.o: threading.h analysis.h priorityqueue.h index.h except.h hash.h multimapper.h posh.h mempool.h search.h similarity.h global.h hashset.h lang.h store.h bitvector.h config.h document.h

test_multimapper.o: hash.h except.h multimapper.h posh.h testhelper.h test.h global.h lang.h config.h

test_similarity.o: except.h posh.h test.h global.h similarity.h lang.h config.h

bitvector.o: except.h posh.h global.h lang.h config.h bitvector.h

test_document.o: hash.h except.h posh.h test.h global.h lang.h config.h document.h

test_q_filtered.o: threading.h analysis.h priorityqueue.h index.h hash.h except.h multimapper.h posh.h mempool.h test.h search.h similarity.h hashset.h global.h lang.h store.h bitvector.h document.h config.h

test_q_const_score.o: threading.h analysis.h priorityqueue.h index.h hash.h except.h multimapper.h posh.h mempool.h test.h search.h similarity.h hashset.h global.h lang.h store.h bitvector.h document.h config.h

test_filter.o: threading.h analysis.h priorityqueue.h index.h hash.h except.h multimapper.h posh.h mempool.h testhelper.h test.h search.h similarity.h hashset.h global.h lang.h store.h bitvector.h document.h config.h

store.o: threading.h hash.h except.h posh.h hashset.h global.h lang.h store.h config.h

ram_store.o: threading.h hash.h except.h posh.h hashset.h global.h lang.h store.h config.h

q_parser.o: threading.h analysis.h priorityqueue.h index.h except.h hash.h multimapper.h posh.h mempool.h search.h similarity.h global.h hashset.h lang.h array.h store.h bitvector.h config.h document.h

test_file_deleter.o: priorityqueue.h analysis.h threading.h hash.h except.h index.h multimapper.h posh.h mempool.h testhelper.h test.h similarity.h hashset.h global.h lang.h store.h bitvector.h document.h config.h

test_search.o: threading.h analysis.h priorityqueue.h index.h hash.h except.h multimapper.h posh.h mempool.h testhelper.h test.h search.h similarity.h hashset.h global.h lang.h array.h helper.h store.h bitvector.h document.h config.h

mempool.o: except.h posh.h mempool.h global.h lang.h config.h

q_fuzzy.o: threading.h analysis.h priorityqueue.h index.h except.h hash.h multimapper.h posh.h mempool.h search.h similarity.h global.h hashset.h lang.h helper.h store.h bitvector.h config.h document.h

hash.o: except.h hash.h posh.h global.h lang.h config.h

compound_io.o: priorityqueue.h analysis.h threading.h hash.h except.h index.h multimapper.h posh.h mempool.h similarity.h hashset.h global.h lang.h array.h store.h bitvector.h document.h config.h

test_q_span.o: threading.h analysis.h priorityqueue.h index.h hash.h except.h multimapper.h posh.h mempool.h test.h search.h similarity.h hashset.h global.h lang.h store.h bitvector.h document.h config.h

test_compound_io.o: priorityqueue.h analysis.h threading.h hash.h except.h index.h multimapper.h posh.h mempool.h testhelper.h test.h similarity.h hashset.h global.h lang.h store.h bitvector.h document.h config.h

test_hashset.o: hash.h except.h posh.h test.h global.h hashset.h lang.h config.h

filter.o: threading.h analysis.h priorityqueue.h index.h except.h hash.h multimapper.h posh.h mempool.h search.h similarity.h global.h hashset.h lang.h store.h bitvector.h config.h document.h

q_const_score.o: threading.h analysis.h priorityqueue.h index.h except.h hash.h multimapper.h posh.h mempool.h search.h similarity.h global.h hashset.h lang.h store.h bitvector.h config.h document.h

search.o: threading.h analysis.h priorityqueue.h index.h except.h hash.h multimapper.h posh.h mempool.h search.h similarity.h global.h hashset.h lang.h array.h store.h bitvector.h config.h document.h

test_test.o: except.h posh.h test.h global.h lang.h config.h

multimapper.o: hash.h except.h multimapper.h posh.h global.h lang.h array.h config.h bitvector.h

q_span.o: threading.h analysis.h priorityqueue.h index.h except.h hash.h multimapper.h posh.h mempool.h search.h similarity.h global.h hashset.h lang.h store.h bitvector.h config.h document.h

test_sort.o: threading.h analysis.h priorityqueue.h index.h hash.h except.h multimapper.h posh.h mempool.h testhelper.h test.h search.h similarity.h hashset.h global.h lang.h store.h bitvector.h document.h config.h

test_fields.o: priorityqueue.h analysis.h threading.h hash.h except.h index.h multimapper.h posh.h mempool.h test.h similarity.h hashset.h global.h lang.h store.h bitvector.h document.h config.h

test_bitvector.o: except.h posh.h testhelper.h test.h global.h lang.h config.h bitvector.h

test_store.o: threading.h hash.h except.h posh.h test.h hashset.h global.h lang.h store.h config.h

posh.o: posh.h

term_vectors.o: priorityqueue.h analysis.h threading.h hash.h except.h index.h multimapper.h posh.h mempool.h similarity.h hashset.h global.h lang.h array.h helper.h store.h bitvector.h document.h config.h

fs_store.o: threading.h hash.h except.h posh.h hashset.h global.h lang.h store.h config.h

index.o: analysis.h threading.h priorityqueue.h hash.h except.h index.h multimapper.h posh.h mempool.h hashset.h global.h similarity.h lang.h helper.h array.h store.h bitvector.h document.h config.h

test_threading.o: threading.h analysis.h priorityqueue.h index.h except.h hash.h multimapper.h posh.h ind.h mempool.h testhelper.h test.h search.h global.h hashset.h similarity.h lang.h store.h config.h document.h bitvector.h

test_ram_store.o: threading.h hash.h except.h posh.h test.h hashset.h global.h test_store.h lang.h store.h config.h

test.o: all_tests.h except.h posh.h test.h global.h lang.h config.h

ind.o: threading.h analysis.h priorityqueue.h index.h except.h hash.h multimapper.h posh.h ind.h mempool.h search.h global.h hashset.h similarity.h lang.h array.h store.h config.h document.h bitvector.h

q_range.o: threading.h analysis.h priorityqueue.h index.h except.h hash.h multimapper.h posh.h mempool.h search.h similarity.h global.h hashset.h lang.h store.h bitvector.h config.h document.h

sort.o: priorityqueue.h analysis.h threading.h hash.h except.h index.h multimapper.h posh.h mempool.h search.h similarity.h hashset.h global.h lang.h store.h bitvector.h document.h config.h

q_match_all.o: threading.h analysis.h priorityqueue.h index.h except.h hash.h multimapper.h posh.h mempool.h search.h similarity.h global.h hashset.h lang.h store.h bitvector.h config.h document.h

test_fs_store.o: threading.h hash.h except.h posh.h test.h hashset.h global.h test_store.h lang.h store.h config.h

test_segments.o: priorityqueue.h analysis.h threading.h hash.h except.h index.h multimapper.h posh.h mempool.h test.h similarity.h hashset.h global.h lang.h store.h bitvector.h document.h config.h

test_q_parser.o: threading.h analysis.h priorityqueue.h index.h hash.h except.h multimapper.h posh.h mempool.h test.h search.h similarity.h hashset.h global.h lang.h store.h bitvector.h document.h config.h

test_global.o: except.h posh.h test.h global.h lang.h config.h

q_boolean.o: threading.h analysis.h priorityqueue.h index.h except.h hash.h multimapper.h posh.h mempool.h search.h similarity.h global.h hashset.h lang.h array.h store.h bitvector.h config.h document.h

array.o: except.h posh.h global.h lang.h array.h config.h

test_term.o: priorityqueue.h analysis.h threading.h hash.h except.h index.h multimapper.h posh.h mempool.h test.h similarity.h hashset.h global.h lang.h store.h bitvector.h document.h config.h

global.o: except.h posh.h global.h lang.h config.h

analysis.o: analysis.h except.h hash.h multimapper.h posh.h global.h lang.h config.h

test_priorityqueue.o: priorityqueue.h except.h posh.h test.h global.h lang.h config.h

test_helper.o: except.h posh.h test.h global.h lang.h helper.h config.h

test_mempool.o: except.h posh.h mempool.h test.h global.h lang.h config.h

test_index.o: priorityqueue.h analysis.h threading.h hash.h except.h index.h multimapper.h posh.h mempool.h testhelper.h test.h similarity.h hashset.h global.h lang.h store.h bitvector.h document.h config.h

document.o: hash.h except.h posh.h global.h lang.h config.h document.h

test_analysis.o: analysis.h hash.h except.h multimapper.h posh.h test.h global.h lang.h config.h

test_highlighter.o: threading.h analysis.h priorityqueue.h index.h hash.h except.h multimapper.h posh.h mempool.h testhelper.h test.h search.h similarity.h hashset.h global.h lang.h array.h store.h bitvector.h document.h config.h

test_q_fuzzy.o: threading.h analysis.h priorityqueue.h index.h hash.h except.h multimapper.h posh.h mempool.h test.h search.h similarity.h hashset.h global.h lang.h store.h bitvector.h document.h config.h

q_filtered_query.o: threading.h analysis.h priorityqueue.h index.h except.h hash.h multimapper.h posh.h mempool.h search.h similarity.h global.h hashset.h lang.h store.h bitvector.h config.h document.h

test_except.o: except.h posh.h test.h global.h lang.h config.h

helper.o: posh.h helper.h config.h

test_array.o: except.h posh.h test.h global.h lang.h array.h config.h

hashset.o: hash.h except.h posh.h global.h hashset.h lang.h config.h

q_wildcard.o: threading.h analysis.h priorityqueue.h index.h except.h hash.h multimapper.h posh.h mempool.h search.h similarity.h global.h hashset.h lang.h store.h bitvector.h config.h document.h

