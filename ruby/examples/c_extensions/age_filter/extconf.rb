# extconf.rb for Ferret extensions
require 'rubygems'
require 'mkmf'
$INCFLAGS = "-I#{File.join(Gem.cache.search('ferret').last.full_gem_path, 'ext')} #{$INCFLAGS}"
create_makefile("age_filter")
