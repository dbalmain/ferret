#!/usr/bin/env ruby

require 'set'

$dep_tree = {}
$files = Dir["src/*.c"] + Dir["include/*.h"] + Dir["test/*.[ch]"]
$files.each do |file_name|
  File.open(file_name) do |file|
    file.each do |line|
      if line =~ /#include "(.*)"/
        ($dep_tree[File.basename(file_name)] ||= Set.new) << $1
      end
    end
  end
end

objs = Dir["src/*.c"].map {|full_path| File.basename(full_path).gsub(/c$/, "o")}
test_objs = Dir["test/*.c"].map {|full_path| File.basename(full_path).gsub(/c$/, "o")}

print """
CFLAGS = -std=c99 -pedantic -Wall -Wextra -Iinclude -Ilib/libstemmer_c/include -fno-common -g -DDEBUG #-D_FILE_OFFSET_BITS=64

LFLAGS = -lm -lpthread

include lib/libstemmer_c/mkinc.mak

STEMMER_OBJS = $(patsubst %.c,src/libstemmer_c/%.o, $(snowball_sources))

TEST_OBJS = #{test_objs.join(" ")}

OBJS = #{objs.join(" ")} libstemmer.o

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

"""

def get_deps(src)
  deps = Set.new
  direct_deps = $dep_tree[src]
  return deps unless direct_deps
  deps.merge(direct_deps)
  direct_deps.each {|dep| deps.merge(get_deps(dep))}
  deps
end

$dep_tree.each_key do |src|
  next unless src =~ /\.c$/
  puts src.gsub(/c$/, "o") + ": " + get_deps(src).to_a.join(" ") + "\n\n"
end
