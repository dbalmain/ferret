$:.unshift File.join(File.dirname(__FILE__), '../ext')

# Author _Dave_ _Balmain_ (based loosely on Doug Cutting's Lucene)
module Ferret
  VERSION="0.1"
end

require 'utils'
require 'document'
require 'stemmers'
require 'analysis'
require 'store'
require 'index'
require 'search'
require 'query_parser'
if File.exist?(File.join(File.dirname(__FILE__), '../ext/extensions.so'))
  require 'extensions'
end
