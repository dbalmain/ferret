
CFLAGS = -std=c99 -pedantic -Wall -Wextra -Iinclude -Ilib/libstemmer_c/include -fno-common -O2 -g -DDEBUG

LFLAGS = -lm -lpthread

include lib/libstemmer_c/mkinc.mak

STEMMER_OBJS = $(patsubst %.c,src/libstemmer_c/%.o, $(snowball_sources))

TEST_OBJS = test_priorityqueue.o test_hashset.o test_helper.o test_test.o test.o test_global.o test_bitvector.o test_hash.o test_ram_store.o test_store.o test_fs_store.o test_except.o test_document.o test_fields.o test_analysis.o

OBJS = priorityqueue.o hashset.o helper.o global.o bitvector.o hash.o fs_store.o posh.o except.o ram_store.o store.o analysis.o fields.o document.o stopwords.o libstemmer.o

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

test_ram_store.o: threading.h defines.h hash.h except.h posh.h test.h global.h test_store.h store.h

test_store.o: threading.h defines.h hash.h except.h posh.h test.h global.h store.h

test_bitvector.o: defines.h except.h posh.h test.h global.h bitvector.h

test_test.o: defines.h except.h posh.h test.h global.h

global.o: defines.h except.h posh.h global.h

hashset.o: defines.h hash.h except.h posh.h global.h hashset.h

ram_store.o: threading.h defines.h hash.h except.h posh.h global.h store.h

helper.o: defines.h posh.h helper.h

store.o: threading.h defines.h hash.h except.h posh.h global.h store.h

test_fields.o: defines.h threading.h hash.h except.h fields.h posh.h test.h global.h store.h

test_except.o: defines.h except.h posh.h test.h global.h

test_hash.o: defines.h except.h hash.h posh.h test.h global.h

test_global.o: defines.h except.h posh.h test.h global.h

test_helper.o: defines.h except.h posh.h test.h global.h helper.h

document.o: defines.h hash.h except.h posh.h global.h document.h

bitvector.o: defines.h except.h posh.h global.h bitvector.h

test_fs_store.o: threading.h defines.h hash.h except.h posh.h test.h global.h test_store.h store.h

analysis.o: defines.h analysis.h except.h hash.h posh.h global.h

priorityqueue.o: defines.h priorityqueue.h except.h posh.h global.h

fields.o: defines.h threading.h hash.h except.h fields.h posh.h global.h store.h

except.o: defines.h threading.h except.h posh.h global.h

hash.o: defines.h except.h hash.h posh.h global.h

test_analysis.o: defines.h analysis.h hash.h except.h posh.h test.h global.h

fs_store.o: threading.h defines.h hash.h except.h posh.h global.h store.h

posh.o: posh.h

test_hashset.o: defines.h hash.h except.h posh.h test.h global.h hashset.h

test_priorityqueue.o: defines.h priorityqueue.h except.h posh.h test.h global.h


