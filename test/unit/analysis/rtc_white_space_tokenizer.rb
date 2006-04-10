require File.dirname(__FILE__) + "/../../test_helper"

class WhiteSpaceTokenizerTest < Test::Unit::TestCase
  include Ferret::Analysis
  include Ferret::Utils::StringHelper

  def test_whitespacetokenizer()
    input = StringReader.new('DBalmain@gmail.com is My e-mail 52   #$ address. 23#@$')
    t = WhiteSpaceTokenizer.new(input)
    assert_equal(Token.new('DBalmain@gmail.com', 0, 18), t.next())
    assert_equal(Token.new('is', 19, 21), t.next())
    assert_equal(Token.new('My', 22, 24), t.next())
    assert_equal(Token.new('e-mail', 25, 31), t.next())
    assert_equal(Token.new('52', 32, 34), t.next())
    assert_equal(Token.new('#$', 37, 39), t.next())
    assert_equal(Token.new('address.', 40, 48), t.next())
    assert_equal(Token.new('23#@$', 49, 54), t.next())
    assert(! t.next())
  end
end
