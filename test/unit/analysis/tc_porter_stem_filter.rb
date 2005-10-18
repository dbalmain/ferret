require File.dirname(__FILE__) + "/../../test_helper"

class PorterStemFilterTest < Test::Unit::TestCase
  include Ferret::Analysis
  include Ferret::Utils::StringHelper

  def test_porterstempfilter()
    input = StringReader.new('breath Breathes BreatHed BREATHING')
    t = PorterStemFilter.new(LowerCaseFilter.new(WhiteSpaceTokenizer.new(input)))
    assert_equal(Token.new('breath', 0, 6), t.next())
    assert_equal(Token.new('breath', 7, 15), t.next())
    assert_equal(Token.new('breath', 16, 24), t.next())
    assert_equal(Token.new('breath', 25, 34), t.next())
    assert(! t.next())
  end
end
