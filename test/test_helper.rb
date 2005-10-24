$:.unshift File.join(File.dirname(__FILE__), '../lib')
$:.unshift File.join(File.dirname(__FILE__), '../ext')

require 'test/unit'
require 'ferret'
require 'test/unit/index/th_doc'

def load_test_dir(dir)
  dir = File.join(File.dirname(__FILE__), dir)
  Dir.foreach(dir) do |file|
    require File.join(dir, file) if file =~ /^t[mcs]_.*\.rb$/
  end
end
