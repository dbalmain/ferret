require File.dirname(__FILE__) + "/../../test_helper"

include Ferret::Index

class TermTest < Test::Unit::TestCase
  def test_term()
    term1 = Term.new("orange", "A citrus fruit")
    assert_equal(term1.field_name, "orange")
    assert_equal(term1.text, "A citrus fruit")
    term2 = Term.new("lemon", "A citrus fruit")
    term3 = Term.new("orange", "A delicious treat")
    term4 = Term.new("orange", "A citrus fruit")
    assert(term1 > term2)
    assert(term1 < term3)
    assert(term1.between?(term2, term3))
    assert(term1 == term4)
    assert(term1.eql?(term4))
    term4.set!("apple", "An apple a day keeps the dr away")
    assert(term1 != term4)
  end

end

class TermDataTest < Test::Unit::TestCase
  def test_add_data
    td = TermData.new
    td.add_data(1, 2, 4, 8)
    td.add_data(1, 9, 45, 49)
    td.add_data(8, 20, 87, 92)
    td.add_data(29, 5, 23, 28)
    td.add_data(29, 20, 87, 92)
    td.add_data(29, 200, 1009, 1014)
    assert_equal([1,8,29], td.documents)
    assert_equal([[2,9], [20], [5,20,200]], td.positions)
    assert_equal([2,1,3], td.frequencies)
    assert_equal([[[4,8],[45,49]],[[87,92]],[[23,28],[87,92],[1009,1014]]],
                 td.offsets)

    assert_equal([5,20,200], td.positions_in_doc(29))
    assert_equal(2, td.frequency_in_doc(1))
    assert_equal([[23,28],[87,92],[1009,1014]], td.offsets_in_doc(29))
  end
end
