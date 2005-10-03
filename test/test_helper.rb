$:.unshift File.join(File.dirname(__FILE__), '../lib')

require 'test/unit'
require 'test/unit/index/th_doc'
require 'ferret'

include Ferret::Utils::StringHelper
