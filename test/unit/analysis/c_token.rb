require File.dirname(__FILE__) + "/../../test_helper"

class TokenTest < Test::Unit::TestCase
  include Ferret::Analysis

  def test_token()
    tk1 = Token.new("DBalmain", 1, 8, 5, "token")
    assert_equal(tk1, Token.new("DBalmain", 1, 8))
    assert_not_equal(tk1, Token.new("DBalmain", 0, 8))
    assert_not_equal(tk1, Token.new("DBalmain", 1, 9))
    assert_not_equal(tk1, Token.new("Dbalmain", 1, 8))
    assert(tk1 < Token.new("CBalmain", 2, 7))
    assert(tk1 > Token.new("EBalmain", 0, 9))
    assert(tk1 < Token.new("CBalmain", 1, 9))
    assert(tk1 > Token.new("EBalmain", 1, 7))
    assert(tk1 < Token.new("EBalmain", 1, 8))
    assert(tk1 > Token.new("CBalmain", 1, 8))
    assert_equal("DBalmain", tk1.text)
    tk1.text = "Hello"
    assert_equal("Hello", tk1.text)
    assert_equal(1, tk1.start_offset)
    assert_equal(8, tk1.end_offset)
    assert_equal(5, tk1.pos_inc)
  end
end
