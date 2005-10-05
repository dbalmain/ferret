require File.dirname(__FILE__) + "/../../test_helper"
include Ferret::Analysis

class StandardAnalyzerTest < Test::Unit::TestCase
  def test_lettertokenizer()
    input = StringReader.new('D.Ba_l-n@gma-l.com AB&Sons Toys\'r\'us you\'re she\'s, #$%^$%*& job@dot I.B.M. the an AnD THEIR')
    sa = StandardAnalyzer.new()
    t = sa.token_stream("field", input)
    assert_equal(Token.new("d.ba_l-n@gma-l.com", 0, 18), t.next())
    assert_equal(Token.new("ab&sons", 19, 26), t.next())
    assert_equal(Token.new("toys'r'us", 27, 36), t.next())
    assert_equal(Token.new("you're", 37, 43), t.next())
    assert_equal(Token.new("she", 44, 49), t.next())
    assert_equal(Token.new("job@dot", 60, 67), t.next())
    assert_equal(Token.new("ibm", 68, 74), t.next())
    assert(! t.next())
  end
end