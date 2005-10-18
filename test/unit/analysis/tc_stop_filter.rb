require File.dirname(__FILE__) + "/../../test_helper"

class StopFilterTest < Test::Unit::TestCase
  include Ferret::Analysis
  include Ferret::Utils::StringHelper

  def test_stopfilter()
    input = StringReader.new('The Quick AND the DEAD the and to it there their')
    t = StopFilter.new_with_file(LowerCaseTokenizer.new(input), File.dirname(__FILE__) + '/data/wordfile')
    assert_equal(Token.new('quick', 4, 9), t.next())
    assert_equal(Token.new('dead', 18, 22), t.next())
    assert(! t.next())
  end
end
