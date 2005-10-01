require File.dirname(__FILE__) + "/../../test_helper"

include Ferret::Index

class SegmentTermVectorTest < Test::Unit::TestCase
  def setup()
    @terms = ["Apples", "Oranges", "Bananas", "Kiwis", "Mandarins"]
    term_freqs = [4,2,1,12,4]
    @stv = SegmentTermVector.new("Fruits", @terms, term_freqs)
  end

  def test_size()
    assert_equal(@terms.size(), @stv.size())
  end

  def test_index_of()
    assert_equal(0, @stv.index_of("Apples"))
    assert_equal(4, @stv.term_frequencies[@stv.index_of("Apples")])
  end

  def test_indexes_of()
    assert_equal([2, 0, 3], @stv.indexes_of(["Bananas", "Apples", "Kiwis"], 0, 3))
    assert_equal([0, 3], @stv.indexes_of(["Bananas", "Apples", "Kiwis"], 1, 2))
  end
end
