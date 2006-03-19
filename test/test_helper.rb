$:.unshift File.dirname(__FILE__)
$:.unshift File.join(File.dirname(__FILE__), '../lib')
$:.unshift File.join(File.dirname(__FILE__), '../ext')

class Float
  def =~(o)
    return (1 - self/o).abs < 0.00001
  end
end

require 'test/unit'
require 'unit/index/th_doc'
if $ferret_pure_ruby
  require 'rferret'
else
  require 'ferret'
end

def load_test_dir(dir)
  dir = File.join(File.dirname(__FILE__), dir)
  Dir.foreach(dir) do |file|
    if $ferret_pure_ruby
      require File.join(dir, file) if file =~ /^t?[mcs]_.*\.rb$/
    else
      require File.join(dir, file) if file =~ /^[mcs]_.*\.rb$/
    end
  end
end
