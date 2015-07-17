###
# Ferret Makefile
#
# Common CFLAGS options
#  -D_FILE_OFFSET_BITS=64     // set this option for large-file support
#  -g                         // set -g for debugging purposes
#  -pg                        // use this option if you want to use gprof
#  -DDEBUG                    // some error checking only occurs in DEBUG mode
#  -DHAVE_GDB                 // defined on platforms that have gdb
#  -DBZ_NO_STDIO              // There really is no need for bzlib to use stdio
#  -DUSE_ZLIB                 // use zlib instead of bzlib
#  -D_POSIX_C_SOURCE=2        // for popen which is used to print stacktraces
#  -fprofile-arcs -ftest-coverage // for gcov
###

STEMMER_DIR = lib/libstemmer_c
STEMMER_INC = $(STEMMER_DIR)/include

BZLIB_DIR = lib/bzlib
BZLIB_INC = $(BZLIB_DIR)

CC       = gcc
CINCS    = -Iinclude -I$(STEMMER_INC) -I$(BZLIB_INC) -Itest
DEFS     = -DBZ_NO_STDIO -D_FILE_OFFSET_BITS=64 -DDEBUG -D_POSIX_C_SOURCE=2
DEFS    += -DHAVE_GDB
CFLAGS   = -std=c99 -pedantic -Wall -Wextra $(CINCS) -g -fno-common $(DEFS)
LDFLAGS  = -lm -lpthread -lz
DEP_DIR  = .deps
GCOV_DIR = .gcov
VPATH    = test:src:benchmark

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
store.o             term_vectors.o       field_index.o      lang.o            \
scanner.o           scanner_mb.o         symbol.o           scanner_utf8.o

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
test_sort.o              test_ram_store.o         test_file_deleter.o  \
test_lang.o              test_symbol.o            test_1710.o

BENCH_OBJS = benchmark.o bm_bitvector.o bm_hash.o \
             bm_micro_string.o bm_store.o         \

BZLIB_SRCS = \
bzlib.c blocksort.c compress.c crctable.c decompress.c huffman.c randtable.c
BZLIB_OBJS = $(BZLIB_SRCS:%.c=$(BZLIB_DIR)/%.o)

include $(STEMMER_DIR)/mkinc.mak
STEMMER_OBJS = $(snowball_sources:%.c=$(STEMMER_DIR)/%.o)

FRT_OBJS = $(OBJS) $(TEST_OBJS) $(BENCH_OBJS)
EXT_OBJS = $(BZLIB_OBJS) $(STEMMER_OBJS)

###
# Common Targets
###

all: libferret.a

libferret.a: $(OBJS) $(BZLIB_OBJS) $(STEMMER_OBJS)
	@echo Generating library: $@ ...
	@$(AR) -cru $@ $^

test: testall
	@rm -f *.gcda
	@./testall -q -v

testall: $(TEST_OBJS) libferret.a
	@echo Building task: $@ ...
	@$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

benchall: $(BENCH_OBJS) libferret.a
	@echo Building task: $@ ...
	@$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

bench: benchall
	@./benchall

clean:
	@rm -f  $(FRT_OBJS) $(FRT_OBJS:%.o=%.gcov) $(FRT_OBJS:%.o=%.gcno)
	@rm -f  libferret.a testall gmon.out
	@rm -rf $(DEP_DIR) $(GCOV_DIR)

realclean: clean
	@rm -f $(EXT_OBJS) $(EXT_OBJS:%.o=%.gcov) $(EXT_OBJS:%.o=%.gcno)

###
# Profiling + coverage
# Note: Make clean before using these for the first time.
###

gcov: CFLAGS += -fprofile-arcs -ftest-coverage
gcov: test $(OBJS:%.o=%.c.gcov)
	@head -1 $(OBJS:%.o=$(GCOV_DIR)/%.c.gcov.out) \
             | grep -v '==>' | egrep -v '^[ ]*$$' | sort -rn -k 2
	@rm -f test*gcda test*gcno

gprof: CFLAGS += -pg
gprof: test

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
# Dependency build
# Ref: http://make.paulandlesley.org/autodep.html
###

%.o : %.c
	@mkdir -p `dirname $(DEP_DIR)/$*.P`
	@echo Compiling: $< ...
	@$(COMPILE.c) -MD -o $@ $<
	@cp $*.d $(DEP_DIR)/$*.P; \
            sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
                -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $(DEP_DIR)/$*.P; \
            rm -f $*.d

-include $(FRT_OBJS:%.o=$(DEP_DIR)/%.P)

src/q_parser.c: src/q_parser.y

%.c : %.rl
	ragel $< -o $@

%.c : %.y
	yacc -o $@ -p frt_ $<

###
# Generate gcov stats
###

%.c.gcov : %.o
	@mkdir -p $(GCOV_DIR)
	@if [ -e $(*F).gcda ]; then mv $(*F).gcda $(GCOV_DIR); fi
	@if [ -e $(*F).gcno ]; then mv $(*F).gcno $(GCOV_DIR); fi
	@echo Generating coverage for: $< ...
	@-if [ -e $< ] ; then \
             echo -n $<: " " > $(GCOV_DIR)/$@.out ;                          \
             gcov -o $(GCOV_DIR) $<                                          \
                 | head -2         | tail -1                                 \
                 | cut -d ':' -f 2 | cut -d '%' -f 1 >> $(GCOV_DIR)/$@.out ; \
             if [ -e $@ ] ; then mv $@ $(GCOV_DIR) ; fi ;                    \
         fi

###
# Etc
###

.PHONY: clean all test bench gcov gprof valgrind gen_valsupp
