require File.dirname(__FILE__) + "/../../test_helper"

include Ferret::Index

class TermVectorOffsetInfoTest < Test::Unit::TestCase
  def test_tvoi()
    t1 = TermVectorOffsetInfo.new(1, 3)
    assert_equal(t1.start_offset, 1)
    assert_equal(t1.end_offset, 3)
    t2 = TermVectorOffsetInfo.new(1, 3)
    assert(t1 == t2)
    t2.start_offset = 2
    assert(t1 != t2)
    t2.start_offset = 1
    t2.end_offset = 1
    assert(t1 != t2)
  end
end
