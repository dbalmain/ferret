
CFLAGS = -std=c99 -pedantic -Wall -Wextra -Iinclude -Ilib/libstemmer_c/include -fno-common -O2 -g -DDEBUG

LFLAGS = -lm -lpthread

include lib/libstemmer_c/mkinc.mak

STEMMER_OBJS = $(patsubst %.c,src/libstemmer_c/%.o, $(snowball_sources))

TEST_OBJS = test_priorityqueue.o test_hashset.o test_helper.o test_test.o test.o test_global.o test_bitvector.o test_hash.o test_ram_store.o test_store.o test_term_vectors.o test_fs_store.o test_except.o test_document.o test_fields.o test_segments.o test_analysis.o test_term.o test_array.o test_similarity.o test_mem_pool.o testhelper.o test_index.o test_compound_io.o

OBJS = priorityqueue.o hashset.o helper.o global.o bitvector.o hash.o fs_store.o posh.o except.o ram_store.o store.o analysis.o document.o similarity.o stopwords.o array.o index.o mem_pool.o compound_io.o libstemmer.o

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

test_document.o: defines.h hash.h except.h posh.h test.h global.h document.h

test.o: defines.h all_tests.h except.h posh.h test.h global.h

test_mem_pool.o: defines.h mem_pool.h except.h posh.h test.h global.h

test_array.o: defines.h except.h posh.h test.h global.h array.h

test_ram_store.o: threading.h defines.h hash.h except.h posh.h test.h global.h test_store.h store.h

test_term_vectors.o: mem_pool.h analysis.h defines.h threading.h hash.h except.h index.h posh.h test.h similarity.h hashset.h global.h store.h bitvector.h document.h

test_store.o: threading.h defines.h hash.h except.h posh.h test.h global.h store.h

test_bitvector.o: defines.h except.h posh.h test.h global.h bitvector.h

test_test.o: defines.h except.h posh.h test.h global.h

similarity.o: defines.h posh.h similarity.h helper.h

global.o: defines.h except.h posh.h global.h

hashset.o: defines.h hash.h except.h posh.h global.h hashset.h

test_term.o: mem_pool.h analysis.h defines.h threading.h hash.h except.h index.h posh.h test.h similarity.h hashset.h global.h store.h bitvector.h document.h

array.o: defines.h except.h posh.h global.h array.h

ram_store.o: threading.h defines.h hash.h except.h posh.h global.h store.h

helper.o: defines.h posh.h helper.h

compound_io.o: mem_pool.h analysis.h defines.h threading.h hash.h except.h index.h posh.h similarity.h hashset.h global.h array.h store.h bitvector.h document.h

store.o: threading.h defines.h hash.h except.h posh.h global.h store.h

test_fields.o: mem_pool.h analysis.h defines.h threading.h hash.h except.h index.h posh.h test.h similarity.h hashset.h global.h store.h bitvector.h document.h

test_except.o: defines.h except.h posh.h test.h global.h

test_hash.o: defines.h except.h hash.h posh.h test.h global.h

test_global.o: defines.h except.h posh.h test.h global.h

test_helper.o: defines.h except.h posh.h test.h global.h helper.h

mem_pool.o: defines.h mem_pool.h except.h posh.h global.h

document.o: defines.h hash.h except.h posh.h global.h document.h

bitvector.o: defines.h except.h posh.h global.h bitvector.h

test_compound_io.o: mem_pool.h analysis.h defines.h threading.h hash.h except.h index.h posh.h test.h similarity.h hashset.h global.h test_store.h store.h bitvector.h document.h

test_segments.o: mem_pool.h analysis.h defines.h threading.h hash.h except.h index.h posh.h test.h similarity.h hashset.h global.h store.h bitvector.h document.h

test_fs_store.o: threading.h defines.h hash.h except.h posh.h test.h global.h test_store.h store.h

analysis.o: defines.h analysis.h except.h hash.h posh.h global.h

priorityqueue.o: defines.h priorityqueue.h except.h posh.h global.h

index.o: mem_pool.h analysis.h defines.h threading.h hash.h except.h index.h posh.h hashset.h global.h similarity.h helper.h array.h store.h bitvector.h document.h

except.o: defines.h threading.h except.h posh.h global.h

hash.o: defines.h except.h hash.h posh.h global.h

test_index.o: mem_pool.h analysis.h defines.h threading.h hash.h except.h index.h posh.h testhelper.h test.h similarity.h hashset.h global.h store.h bitvector.h document.h

test_analysis.o: defines.h analysis.h hash.h except.h posh.h test.h global.h

fs_store.o: threading.h defines.h hash.h except.h posh.h global.h store.h

test_similarity.o: defines.h except.h posh.h test.h global.h similarity.h

posh.o: posh.h

testhelper.o: testhelper.h

test_hashset.o: defines.h hash.h except.h posh.h test.h global.h hashset.h

test_priorityqueue.o: defines.h priorityqueue.h except.h posh.h test.h global.h

