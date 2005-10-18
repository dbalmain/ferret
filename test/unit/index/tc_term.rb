require File.dirname(__FILE__) + "/../../test_helper"


class TermTest < Test::Unit::TestCase
  include Ferret::Index
  def test_term()
    term1 = Term.new("bfield1", "athis is text1")
    assert_equal(term1.field, "bfield1")
    assert_equal(term1.text, "athis is text1")
    term2 = Term.new("afield2", "athis is text1")
    term3 = Term.new("bfield1", "bthis is text2")
    term4 = Term.new("bfield1", "athis is text1")
    assert(term1 > term2)
    assert(term1 < term3)
    assert(term1.between?(term2, term3))
    assert(term1 == term4)
    assert(term1.eql?(term4))
    term4.set!("field3", "text3")
    assert(term1 != term4)
  end

end
