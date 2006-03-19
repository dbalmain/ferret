require File.dirname(__FILE__) + "/../../test_helper"


class TermVectorOffsetInfoTest < Test::Unit::TestCase
  include Ferret::Index
  def test_tvoi()
    t1 = TermVectorOffsetInfo.new(1, 3)
    assert_equal(t1.start, 1)
    assert_equal(t1.end, 3)
    t2 = TermVectorOffsetInfo.new(1, 3)
    assert(t1 == t2)
    t2.start = 2
    assert(t1 != t2)
    t2.start = 1
    t2.end = 1
    assert(t1 != t2)
  end
end
