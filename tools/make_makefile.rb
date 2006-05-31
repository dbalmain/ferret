#!/usr/bin/env ruby

require 'set'

$dep_tree = {}

Dir["**/*.[ch]"].each do |file_name|
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

print """CFLAGS = -std=c99 -pedantic -Wall -Wextra -Iinclude -fno-common -O2 -g -DDEBUG

LFLAGS = -lm -lpthread

TEST_OBJS = #{test_objs.join(" ")}

OBJS = #{objs.join(" ")}

vpath %.c test src

vpath %.h test include

runtests: testall
	./testall -v -f -q

testall: $(OBJS) $(TEST_OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) $(OBJS) $(TEST_OBJS) -o testall

valgrind: testall
	valgrind --leak-check=yes -v testall -q

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
