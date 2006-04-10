require File.dirname(__FILE__) + "/../../test_helper"


class NumberToolsTest < Test::Unit::TestCase
  include Ferret::Utils

  def test_near_zero()
    2.times() do |i|
      2.times() { |j| subtest_two_longs(i, j) }
    end
  end

  def test_max()
    # make sure the constants convert to their equivelents
    assert_equal(NumberTools::LONG_MAX_VALUE, NumberTools.s_to_long(NumberTools::MAX_STRING_VALUE))
    assert_equal(NumberTools::MAX_STRING_VALUE, NumberTools.long_to_s(NumberTools::LONG_MAX_VALUE))
    # test near MAX, too

    l = NumberTools::LONG_MAX_VALUE
    subtest_two_longs(l, l - 1)
  end

  def test_min()
    # make sure the constants convert to their equivelents
    assert_equal(NumberTools::LONG_MIN_VALUE, NumberTools.s_to_long(NumberTools::MIN_STRING_VALUE))
    assert_equal(NumberTools::MIN_STRING_VALUE, NumberTools.long_to_s(NumberTools::LONG_MIN_VALUE))

    # test near MIN, too
    l = NumberTools::LONG_MIN_VALUE
    subtest_two_longs(l, l + 1)
  end

  def subtest_two_longs(i, j)
    # convert to strings
    a = NumberTools.long_to_s(i)
    b = NumberTools.long_to_s(j)

    # are they the right length?
    assert_equal(NumberTools::STR_SIZE, a.length())
    assert_equal(NumberTools::STR_SIZE, b.length())

    # are they the right order?
    if (i < j)
        assert(a < b)
    elsif (i > j)
        assert(a > b)
    else
        assert_equal(a, b)
    end

    # can we convert them back to longs?
    i2 = NumberTools.s_to_long(a)
    j2 = NumberTools.s_to_long(b)

    assert_equal(i, i2)
    assert_equal(j, j2)
  end

end
