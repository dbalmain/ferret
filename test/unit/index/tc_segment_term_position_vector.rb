require File.dirname(__FILE__) + "/../../test_helper"

include Ferret::Index

class SegmentTermPositionVectorTest < Test::Unit::TestCase
  def setup()
    @terms = ["Apples", "Oranges", "Bananas", "Kiwis", "Mandarins"]
    term_freqs = [4,2,1,12,4]
    term_positions = [
      [1,3,5,7],
      [2,4],
      [6],
      [8,9,10,12,13,14,16,17,18,20,21,22],
      [11,15,19,23]
    ]
    term_offsets = [
      [[1,4],[10,14],[20,24],[30,34]],
      [[5,9],[15,19]],
      [[25,29]],
      [[35,39],[40,44],[45,49],[55,59],[60,64],[65,69],[75,79],[80,84],[85,89],[95,99],[100,104],[105,109]],
      [[50,54],[70,74],[90,94],[110,114]]
    ]
    @stv = SegmentTermPositionVector.new("Fruits", @terms, term_freqs, term_positions, term_offsets)
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

  def test_positions_offsets()
    assert_equal([1,3,5,7], @stv.positions[@stv.index_of("Apples")])
    assert_equal([[35,39],[40,44],[45,49],[55,59],[60,64],[65,69],[75,79],[80,84],[85,89],[95,99],[100,104],[105,109]], @stv.offsets[@stv.index_of("Kiwis")])
  end
end
