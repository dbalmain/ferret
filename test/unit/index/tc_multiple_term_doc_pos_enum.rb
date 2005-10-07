require File.dirname(__FILE__) + "/../../test_helper"

include Ferret::Search
include Ferret::Store
include Ferret::Analysis

class MultipleTermDocPosEnumTest < Test::Unit::TestCase
  def setup()
    @dir = RAMDirectory.new()
    iw = IndexWriter.new(@dir, WhiteSpaceAnalyzer.new(), true, false)
    @documents = IndexTestHelper.prepare_search_docs()
    @documents.each { |doc| iw << doc }
    iw.close()
    @ir = IndexReader.open(@dir, true)
  end

  def tear_down()
    @ir.close
  end

  def test_mtdpe()
    t1 = Term.new("field", "red")
    t2 = Term.new("field", "brown")
    t3 = Term.new("field", "hairy")
    mtdpe = MultipleTermDocPosEnum.new(@ir, [t1, t2, t3])
    assert(mtdpe.next?)
    assert_equal(1, mtdpe.doc)
    assert_equal(1, mtdpe.freq)
    assert_equal(4, mtdpe.next_position)
    
    assert(mtdpe.next?)
    assert_equal(8, mtdpe.doc)
    assert_equal(1, mtdpe.freq)
    assert_equal(5, mtdpe.next_position)
    
    assert(mtdpe.next?)
    assert_equal(11, mtdpe.doc)
    assert_equal(1, mtdpe.freq)
    assert_equal(4, mtdpe.next_position)
    
    assert(mtdpe.next?)
    assert_equal(14, mtdpe.doc)
    assert_equal(1, mtdpe.freq)
    assert_equal(4, mtdpe.next_position)
    
    assert(mtdpe.next?)
    assert_equal(16, mtdpe.doc)
    assert_equal(3, mtdpe.freq)
    assert_equal(5, mtdpe.next_position)
    assert_equal(7, mtdpe.next_position)
    assert_equal(11, mtdpe.next_position)
    
    assert(mtdpe.next?)
    assert_equal(17, mtdpe.doc)
    assert_equal(2, mtdpe.freq)
    assert_equal(2, mtdpe.next_position)
    assert_equal(7, mtdpe.next_position)
    
    assert(!mtdpe.next?)
    mtdpe.close()
  end

  def test_tp
    tp = @ir.term_positions_for(Term.new("field", "red"))
    assert(tp.next?)
    assert_equal(11, tp.doc)
    assert_equal(1, tp.freq)
    assert_equal(4, tp.next_position)

    assert(tp.next?)
    assert_equal(16, tp.doc)
    assert_equal(1, tp.freq)
    assert_equal(11, tp.next_position)

    assert(tp.next?)
    assert_equal(17, tp.doc)
    assert_equal(1, tp.freq)
    assert_equal(7, tp.next_position)
    tp.close()
  end
end
