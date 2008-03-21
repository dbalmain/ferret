###
# Ferret Makefile
#
# Common CFLAGS options
#  -D_FILE_OFFSET_BITS=64     // set this option for large-file support
#  -g                         // set -g for debugging purposes
#  -pg                        // use this option if you want to use gprof
#  -DDEBUG                    // some error checking only occurs in DEBUG mode
#  -DBZ_NO_STDIO              // There really is no need for bzlib to use stdio
#  -DUSE_ZLIB                 // use zlib instead of bzlib
#  -fprofile-arcs -ftest-coverage // for gcov
###

STEMMER_DIR = lib/libstemmer_c
STEMMER_INC = $(STEMMER_DIR)/include

BZLIB_DIR = lib/bzlib
BZLIB_INC = $(BZLIB_DIR)

CC      = gcc
CINCS   = -Iinclude -I$(STEMMER_INC) -I$(BZLIB_INC)
DEFS    = -DBZ_NO_STDIO -D_FILE_OFFSET_BITS=64 -DDEBUG
CFLAGS  = -std=c99 -pedantic -Wall -Wextra $(CINCS) -g -fno-common $(DEFS)
LDFLAGS = -lm -lpthread -lz
DEP_DIR = .deps
VPATH   = test:src

###
# Objects
###

OBJS = \
analysis.o          array.o              bitvector.o        compound_io.o     \
document.o          except.o             filter.o           fs_store.o        \
global.o            hash.o               hashset.o          helper.o          \
ind.o               index.o              mempool.o          multimapper.o     \
posh.o              priorityqueue.o      q_boolean.o        q_const_score.o   \
q_filtered_query.o  q_fuzzy.o            q_match_all.o      q_multi_term.o    \
q_parser.o          q_phrase.o           q_prefix.o         q_range.o         \
q_span.o            q_term.o             q_wildcard.o       ram_store.o       \
search.o            similarity.o         sort.o             stopwords.o       \
store.o             term_vectors.o

TEST_OBJS = \
test_multimapper.o       test_q_const_score.o     test_threading.o     \
test_q_fuzzy.o           test_term.o              test_q_filtered.o    \
testhelper.o             test_hash.o              test_fs_store.o      \
test_segments.o          test_similarity.o        test_except.o        \
test_store.o             test_highlighter.o       test_hashset.o       \
test_global.o            test_bitvector.o         test_search.o        \
test_fields.o            test_index.o             test_mempool.o       \
test_compound_io.o       test_q_parser.o          test_term_vectors.o  \
test_array.o             test_document.o          test_helper.o        \
test_test.o              test.o                   test_q_span.o        \
test_analysis.o          test_filter.o            test_priorityqueue.o \
test_sort.o              test_ram_store.o         test_file_deleter.o

BZLIB_SRCS = \
bzlib.c blocksort.c compress.c crctable.c decompress.c huffman.c randtable.c
BZLIB_OBJS = $(BZLIB_SRCS:%.c=$(BZLIB_DIR)/%.o)

include $(STEMMER_DIR)/mkinc.mak
STEMMER_OBJS = $(snowball_sources:%.c=$(STEMMER_DIR)/%.o)

###
# Common Targets
###

all: libferret.a

libferret.a: $(OBJS) $(BZLIB_OBJS) $(STEMMER_OBJS)
	$(AR) -cru $@ $^

test: testall
	./testall

testall: $(TEST_OBJS) libferret.a
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

clean:
	@rm -rf *.o *.a *.gcda *.gcov *.gcno testall gmon.out \
                load_bench bench search_bench sort_bench $(DEP_DIR)

###
# Profiling + coverage
###

gcov: CFLAGS += -fprofile-arcs -ftest-coverage
gcov: clean test

gprof: CFLAGS += -pg
gprof: clean test

valgrind: testall
	valgrind --suppressions=valgrind.supp \
                 --leak-check=yes             \
                 --show-reachable=yes         \
                 --workaround-gcc296-bugs=yes -v ./testall -q

gen_valsupp: testall
	valgrind --suppressions=valgrind.supp \
                 --leak-check=yes             \
                 --show-reachable=yes         \
                 --gen-suppressions=yes       \
                 --workaround-gcc296-bugs=yes -v ./testall -q

###
# Benchmarking
###

load_bench bench search_bench sort_bench: libferret.a

###
# Dependency build
# Ref: http://make.paulandlesley.org/autodep.html
###

%.o : %.c
	@mkdir -p $(DEP_DIR)
	$(COMPILE.c) -MD -o $@ $<
	@cp $*.d $(DEP_DIR)/$*.P; \
            sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
                -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $(DEP_DIR)/$*.P; \
            rm -f $*.d

-include $(OBJS:%.o=$(DEP_DIR)/%.P)

###
# Etc
###

.PHONY: clean all test
