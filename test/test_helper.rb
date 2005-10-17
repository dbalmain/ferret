$:.unshift File.join(File.dirname(__FILE__), '../lib')

require 'test/unit'
require 'ferret'
require 'test/unit/index/th_doc'

include Ferret::Utils::StringHelper
