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
objs << "q_parser.o" unless objs.index("q_parser.o")
OBJS = objs.join(' ')
TEST_OBJS = Dir["test/*.c"].map {|full_path| File.basename(full_path).gsub(/c$/, "o")}.join(' ')

def get_deps(src_file)
  deps = Set.new
  direct_deps = $dep_tree[src_file]
  return deps unless direct_deps
  deps.merge(direct_deps)
  direct_deps.each {|dep| deps.merge(get_deps(dep))}
  deps
end

OBJECT_DEPENDENCIES = $dep_tree.keys.map {|src_file|
  next unless src_file =~ /\.c$/
  src_file.gsub(/c$/, "o") + ": " + get_deps(src_file).to_a.join(" ")
}.compact.join("\n\n")

MAKEFILE_TEMPLATE = File.read(File.join(File.dirname(__FILE__), '../Makefile.template'))
puts MAKEFILE_TEMPLATE.gsub(/%([A-Z_]+)%/) {eval($1)}
