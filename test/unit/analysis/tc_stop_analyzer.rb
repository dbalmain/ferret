require File.dirname(__FILE__) + "/../../test_helper"

class StopAnalyzerTest < Test::Unit::TestCase
  include Ferret::Analysis
  include Ferret::Utils::StringHelper

  def test_stopanalyzer()
    input = StringReader.new('The Quick AND the DEAD the and to it there their')
    a = StopAnalyzer.new()
    t = a.token_stream("field name", input)
    assert_equal(Token.new('quick', 4, 9), t.next())
    assert_equal(Token.new('dead', 18, 22), t.next())
    assert(! t.next())
    input = StringReader.new("David Balmain")
    a = StopAnalyzer.new(["david"])
    t = a.token_stream("field name", input)
    assert_equal(Token.new('balmain', 6, 13), t.next())
    assert(! t.next())
  end
end
