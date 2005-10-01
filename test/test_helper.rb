$:.unshift File.join(File.dirname(__FILE__), '../lib')

require 'test/unit'
require 'ferret'

include Ferret::Utils::StringHelper
