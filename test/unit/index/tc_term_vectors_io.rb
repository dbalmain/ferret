require File.dirname(__FILE__) + "/../../test_helper"


class TermVectorsIOTest < Test::Unit::TestCase

  include Ferret::Index

  def setup()
    @dir = Ferret::Store::RAMDirectory.new
    @fis = FieldInfos.new
    @fis.add("field1", true, true, true, true)
    @fis.add("field2", true, true)
  end

  def tear_down()
    @dir.close()
  end

  def test_tv_io_add_fields()
    tv_w = TermVectorsWriter.new(@dir, "_test", @fis)
    tv_w.open_document
    assert(tv_w.document_open?)
    tv_w.open_field("field1")
    tv_w.add_term("text1", 1, [1], [t(0,4)])
    tv_w.add_term("text2", 2, [3,4], [t(5,10), t(11,16)])
    tv_w.close_field()
    tv_w.close_document()
    tv_w.close()

    tv_r = TermVectorsReader.new(@dir, "_test", @fis)
    assert_equal(1, tv_r.size)
    tv = tv_r.get_field_tv(0, "field1")

    assert_equal(2, tv.size)
    assert_equal("text1", tv.terms[0])
    assert_equal(1, tv.term_frequencies[0])
    assert_equal(1, tv.positions[0][0])
    assert_equal(t(0,4), tv.offsets[0][0])

    assert_equal("text2", tv.terms[1])
    assert_equal(2, tv.term_frequencies[1])
    assert_equal(3, tv.positions[1][0])
    assert_equal(t(5,10), tv.offsets[1][0])
    assert_equal(4, tv.positions[1][1])
    assert_equal(t(11,16), tv.offsets[1][1])
    tv_r.close
  end

  def test_tv_io_add_documents()
    tvs1 = [] 
    tvs2 = [] 
    tv = SegmentTermVector.new("field1",
           ["word1", "word2"],
           [3, 2],
           [[1, 5, 8], [2, 9]],
           [[t(0,5), t(34,39), t(45,50)],[t(6,11), t(51,56)]])
    tvs1 << tv
    tv = SegmentTermVector.new("field2",
           ["word3", "word4"],
           [1, 5],
           [[8], [2, 9, 11, 34, 56]],
           [[t(45,50)], [t(6,10), t(51,56), t(64,69), t(103,108), t(183,188)]])
    tvs1 << tv
    tv_w = TermVectorsWriter.new(@dir, "_test", @fis)
    tv = SegmentTermVector.new("field1",
           ["word1", "word2"],
           [3, 2],
           [[1, 5, 8], [2, 9]],
           [[t(0,5), t(34,39), t(45,50)],[t(6,11), t(51,56)]])
    tvs2 << tv
    tv_w.add_all_doc_vectors(tvs1)
    tv_w.add_all_doc_vectors(tvs2)
    tv_w.close
    tv_r = TermVectorsReader.new(@dir, "_test", @fis)
    assert_equal(2, tv_r.size)
    tv = tv_r.get_field_tv(0, "field1")

    assert_equal(2, tv.size)
    assert_equal("word1", tv.terms[0])
    assert_equal(3, tv.term_frequencies[0])
    assert_equal(1, tv.positions[0][0])
    assert_equal(5, tv.positions[0][1])
    assert_equal(8, tv.positions[0][2])
    assert_equal(t(0,5), tv.offsets[0][0])
    assert_equal(t(34,39), tv.offsets[0][1])
    assert_equal(t(45,50), tv.offsets[0][2])

    assert_equal("word2", tv.terms[1])
    assert_equal(2, tv.term_frequencies[1])
    assert_equal(2, tv.positions[1][0])
    assert_equal(9, tv.positions[1][1])
    assert_equal(t(6,11), tv.offsets[1][0])
    assert_equal(t(51,56), tv.offsets[1][1])

    tv = tv_r.get_field_tv(0, "field2")
    assert_equal(2, tv.size)
    assert_equal("word3", tv.terms[0])

    tv = tv_r.get_field_tv(1, "field1")
    assert_equal(2, tv.size)
    assert_equal("word1", tv.terms[0])
  end

  private
    def t(start, finish)
      return TermVectorOffsetInfo.new(start, finish)
    end
end
