require File.dirname(__FILE__) + "/../../test_helper"

class LetterTokenizerTest < Test::Unit::TestCase
  include Ferret::Analysis
  include Ferret::Utils::StringHelper

  def test_lettertokenizer()
    input = StringReader.new('DBalmain@gmail.com is My e-mail 523@#$ address. 23#@$')
    t = LetterTokenizer.new(input)
    assert_equal(Token.new("DBalmain", 0, 8), t.next())
    assert_equal(Token.new("gmail", 9, 14), t.next())
    assert_equal(Token.new("com", 15, 18), t.next())
    assert_equal(Token.new("is", 19, 21), t.next())
    assert_equal(Token.new("My", 22, 24), t.next())
    assert_equal(Token.new("e", 25, 26), t.next())
    assert_equal(Token.new("mail", 27, 31), t.next())
    assert_equal(Token.new("address", 39, 46), t.next())
    assert(! t.next())
  end
end
