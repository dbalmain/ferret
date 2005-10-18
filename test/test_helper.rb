$:.unshift File.join(File.dirname(__FILE__), '../lib')
$:.unshift File.join(File.dirname(__FILE__), '../ext')

require 'test/unit'
require 'ferret'
require 'test/unit/index/th_doc'
