require File.dirname(__FILE__) + "/../../test_helper"


class BitVectorTest < Test::Unit::TestCase
  include Ferret::Utils

  def test_bignum_conversion()
    j = 256
    10.times do
      j *= j
      assert_equal(j, BitVector.string_to_bignum(BitVector.bignum_to_string(j)))
    end
  end

  def test_bv()
    bv = BitVector.new
    assert_equal(0, bv.count)
    bv.set(10)
    assert(bv.get(10))
    assert_equal(1, bv.count)
    bv.set(10)
    assert(bv.get(10))
    assert_equal(1, bv.count)
    bv.set(20)
    assert(bv.get(20))
    assert_equal(2, bv.count)
    bv.set(21)
    assert(bv.get(21))
    assert_equal(3, bv.count)
    bv.clear(21)
    assert(!bv.get(21))
    assert_equal(2, bv.count)
    bv.clear(20)
    assert(!bv.get(20))
    assert_equal(1, bv.count)
    assert(bv.get(10))
  end

  def test_bv_rw()
    dir = Ferret::Store::RAMDirectory.new
    bv = BitVector.new
    assert_equal(0, bv.count)
    bv.set(5)
    assert_equal(1, bv.count)
    bv.set(8)
    assert_equal(2, bv.count)
    bv.set(13)
    assert_equal(3, bv.count)
    bv.set(21)
    assert_equal(4, bv.count)
    bv.set(34)
    assert_equal(5, bv.count)
    bv.write(dir, "bv.test")
    bv = nil
    bv = BitVector.read(dir, "bv.test")
    assert(!bv.get(4))
    assert(bv.get(5))
    assert(!bv.get(6))
    assert(!bv.get(7))
    assert(bv.get(8))
    assert(!bv.get(9))
    assert(!bv.get(12))
    assert(bv.get(13))
    assert(!bv.get(14))
    assert(!bv.get(20))
    assert(bv.get(21))
    assert(!bv.get(22))
    assert(!bv.get(33))
    assert(bv.get(34))
    assert(!bv.get(35))
    assert_equal(5, bv.count)
  end
end
