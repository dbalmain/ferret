require File.dirname(__FILE__) + "/../../test_helper"
include Ferret::Analysis

class PerFieldAnalyzerWrapperTest < Test::Unit::TestCase
  def test_perfieldanalyzerwrapper()
    aw = PerFieldAnalyzerWrapper.new(Analyzer.new())
    aw.add_analyzer("abstract", WhiteSpaceAnalyzer.new())
    aw.add_analyzer("body", StopAnalyzer.new(['is', 'my', 'address']))
    input = StringReader.new('DBalmain@gmail.com is My e-mail ADDRESS')
    t = aw.token_stream("title", input)
    assert_equal(Token.new("dbalmain", 0, 8), t.next())
    assert_equal(Token.new("gmail", 9, 14), t.next())
    assert_equal(Token.new("com", 15, 18), t.next())
    assert_equal(Token.new("is", 19, 21), t.next())
    assert_equal(Token.new("my", 22, 24), t.next())
    assert_equal(Token.new("e", 25, 26), t.next())
    assert_equal(Token.new("mail", 27, 31), t.next())
    assert_equal(Token.new("address", 32, 39), t.next())
    assert(! t.next())
    input.reset()
    t = aw.token_stream("abstract", input)
    assert_equal(Token.new('DBalmain@gmail.com', 0, 18), t.next())
    assert_equal(Token.new('is', 19, 21), t.next())
    assert_equal(Token.new('My', 22, 24), t.next())
    assert_equal(Token.new('e-mail', 25, 31), t.next())
    assert_equal(Token.new("ADDRESS", 32, 39), t.next())
    if ( token = t.next()): puts token.term_text end
    assert(! t.next())
    input.reset()
    t = aw.token_stream("body", input)
    assert_equal(Token.new("dbalmain", 0, 8), t.next())
    assert_equal(Token.new("gmail", 9, 14), t.next())
    assert_equal(Token.new("com", 15, 18), t.next())
    assert_equal(Token.new("e", 25, 26), t.next())
    assert_equal(Token.new("mail", 27, 31), t.next())
    assert(! t.next())
  end
end
