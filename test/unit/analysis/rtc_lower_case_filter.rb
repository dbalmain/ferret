require File.dirname(__FILE__) + "/../../test_helper"

class LowerCaseFilterTest < Test::Unit::TestCase
  include Ferret::Analysis
  include Ferret::Utils::StringHelper

  def test_lowercasefilter()
    input = StringReader.new('DBalmain@gmail.com is My E-Mail 52   #$ ADDRESS. 23#@$')
    t = LowerCaseFilter.new(WhiteSpaceTokenizer.new(input))
    assert_equal(Token.new('dbalmain@gmail.com', 0, 18), t.next())
    assert_equal(Token.new('is', 19, 21), t.next())
    assert_equal(Token.new('my', 22, 24), t.next())
    assert_equal(Token.new('e-mail', 25, 31), t.next())
    assert_equal(Token.new('52', 32, 34), t.next())
    assert_equal(Token.new('#$', 37, 39), t.next())
    assert_equal(Token.new('address.', 40, 48), t.next())
    assert_equal(Token.new('23#@$', 49, 54), t.next())
    assert(! t.next())
  end
end
