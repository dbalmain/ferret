require File.dirname(__FILE__) + "/../../test_helper"
include Ferret::Analysis

class StandardTokenizerTest < Test::Unit::TestCase
  def test_lettertokenizer()
    input = StringReader.new('DBalmain@gmail.com is My e-mail 523@#$ address. 23#@$')
    t = StandardTokenizer.new(input)
    assert_equal(Token.new("DBalmain@gmail.com", 0, 18), t.next())
    assert_equal(Token.new("is", 19, 21), t.next())
    assert_equal(Token.new("My", 22, 24), t.next())
    assert_equal(Token.new("e", 25, 26), t.next())
    assert_equal(Token.new("mail", 27, 31), t.next())
    assert_equal(Token.new("523", 32, 35), t.next())
    assert_equal(Token.new("address", 39, 46), t.next())
    assert_equal(Token.new("23", 48, 50), t.next())
    assert(! t.next())
  end
end
