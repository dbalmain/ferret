require File.dirname(__FILE__) + "/../../test_helper"


class TermInfoTest < Test::Unit::TestCase
  include Ferret::Index
  def test_term()
    ti1 = TermInfo.new(1, 2, 3, 1)
    assert_equal(ti1.doc_freq, 1)
    assert_equal(ti1.freq_pointer, 2)
    assert_equal(ti1.prox_pointer, 3)
    assert_equal(ti1.skip_offset, 1)
    ti2 = ti1.copy_of()
    assert(ti1 == ti2)
    ti2 = TermInfo.new(10, 9, 8)
    assert(ti1 != ti2)
    ti2.set!(ti1)
    assert(ti1 == ti2)
  end
end
