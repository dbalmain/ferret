require File.dirname(__FILE__) + "/../../test_helper"
require File.dirname(__FILE__) + "/th_doc"

include Ferret::Index
include Ferret::Analysis

module IndexReaderTest
  @dir = nil
  @ir = nil

  def test_index_reader
    do_test_term_doc_enum()
    do_test_term_vectors()
    do_test_changing_field()

  end

  def do_test_term_doc_enum()
    assert_equal(IndexTestHelper::IR_TEST_DOC_CNT, @ir.num_docs())
    assert_equal(IndexTestHelper::IR_TEST_DOC_CNT, @ir.max_doc())

    term = Term.new("body", "Wally")
    assert_equal(4, @ir.doc_freq(term))

    tde = @ir.term_docs_for(term)

    assert(tde.next?)
    assert_equal(0, tde.doc())
    assert_equal(1, tde.freq())
    assert(tde.next?)
    assert_equal(5, tde.doc())
    assert_equal(1, tde.freq())
    assert(tde.next?)
    assert_equal(18, tde.doc())
    assert_equal(3, tde.freq())
    assert(tde.next?)
    assert_equal(20, tde.doc())
    assert_equal(6, tde.freq())
    assert_equal(false, tde.next?)

    # test fast read. Use a small array to exercise repeat read
    docs = Array.new(3)
    freqs = Array.new(3)

    term = Term.new("body", "read")
    tde.seek(term)
    assert_equal(3, tde.read(docs, freqs))
    assert_equal([1,2,6], docs)
    assert_equal([1,2,4], freqs)

    assert_equal(3, tde.read(docs, freqs))
    assert_equal([9, 10, 15], docs)
    assert_equal([3, 1, 1], freqs)

    assert_equal(3, tde.read(docs, freqs))
    assert_equal([16, 17, 20], docs)
    assert_equal([2, 1, 1], freqs)

    assert_equal(1, tde.read(docs, freqs))
    assert_equal([21], docs[0, 1])
    assert_equal([6], freqs[0, 1])

    assert_equal(0, tde.read(docs, freqs))

    do_test_term_docpos_enum_skip_to(tde)
    tde.close()

    # test term positions
    term = Term.new("body", "read")
    tde = @ir.term_positions_for(term)
    assert(tde.next?)
    assert_equal(1, tde.doc())
    assert_equal(1, tde.freq())
    assert_equal(3, tde.next_position())

    assert(tde.next?)
    assert_equal(2, tde.doc())
    assert_equal(2, tde.freq())
    assert_equal(1, tde.next_position())
    assert_equal(4, tde.next_position())

    assert(tde.next?)
    assert_equal(6, tde.doc())
    assert_equal(4, tde.freq())
    assert_equal(3, tde.next_position())
    assert_equal(4, tde.next_position())

    assert(tde.next?)
    assert_equal(9, tde.doc())
    assert_equal(3, tde.freq())
    assert_equal(0, tde.next_position())
    assert_equal(4, tde.next_position())

    assert(tde.skip_to(16))
    assert_equal(16, tde.doc())
    assert_equal(2, tde.freq())
    assert_equal(2, tde.next_position())

    assert(tde.skip_to(21))
    assert_equal(21, tde.doc())
    assert_equal(6, tde.freq())
    assert_equal(3, tde.next_position())
    assert_equal(4, tde.next_position())
    assert_equal(5, tde.next_position())
    assert_equal(8, tde.next_position())
    assert_equal(9, tde.next_position())
    assert_equal(10, tde.next_position())

    assert_equal(false, tde.next?)

    do_test_term_docpos_enum_skip_to(tde)
    tde.close()
  end

  def do_test_term_docpos_enum_skip_to(tde)
    term = Term.new("text", "skip")
    tde.seek(term)

    assert(tde.skip_to(10))
    assert_equal(22, tde.doc())
    assert_equal(22, tde.freq())

    assert(tde.skip_to(60))
    assert_equal(60, tde.doc())
    assert_equal(60, tde.freq())

    tde.seek(term)
    assert(tde.skip_to(45))
    assert_equal(45, tde.doc())
    assert_equal(45, tde.freq())

    assert(tde.skip_to(62))
    assert_equal(62, tde.doc())
    assert_equal(62, tde.freq())

    assert(tde.skip_to(63))
    assert_equal(63, tde.doc())
    assert_equal(63, tde.freq())

    assert_equal(false, tde.skip_to(64))

    tde.seek(term)
    assert_equal(false, tde.skip_to(64))
  end

  def t(start_offset, end_offset)
    TermVectorOffsetInfo.new(start_offset, end_offset)
  end

  def do_test_term_vectors()
    tv = @ir.get_term_vector(3, "body")

    assert_equal("body", tv.field)
    assert_equal(["word1", "word2", "word3", "word4"], tv.terms)
    assert_equal([3, 1, 4, 2], tv.term_frequencies)
    assert_equal([[2, 4, 7], [3], [0, 5, 8, 9], [1,6]], tv.positions)
    assert_equal([[t(12,17), t(24,29), t(42,47)],
                  [t(18,23)],
                  [t(0,5), t(30,35), t(48,53), t(54,59)],
                  [t(6,11), t(36,41)]], tv.offsets)
    tv = nil

    tvs = @ir.get_term_vectors(3)
    assert_equal(3, tvs.size)
    tv = tvs[0]
    assert_equal("author", tv.field)
    assert_equal(["Leo", "Tolstoy"], tv.terms)
    assert(tv.offsets.nil?)
    tv = tvs[1]
    assert_equal("body", tv.field)
    assert_equal(["word1", "word2", "word3", "word4"], tv.terms)
    tv = tvs[2]
    assert_equal("title", tv.field)
    assert_equal(["War And Peace"], tv.terms)
    assert(tv.positions.nil?)
    assert_equal(t(0, 13), tv.offsets[0][0])
  end
   
  def do_test_changing_field()
    tv = @ir.get_term_vector(0, "changing_field");
    assert(tv.nil?);

    tv = @ir.get_term_vector(10, "changing_field");
    assert(tv.positions.nil?);
    assert(tv.offsets.nil?);

    tv = @ir.get_term_vector(17, "changing_field");
    assert(tv.positions);
    assert(tv.offsets.nil?);

    tv = @ir.get_term_vector(19, "changing_field");
    assert(tv.positions.nil?);
    assert(tv.offsets);

    tv = @ir.get_term_vector(20, "changing_field");
    assert(tv.positions);
    assert(tv.offsets);

    tv = @ir.get_term_vector(21, "changing_field");
    assert(tv.nil?);
  end
end

class SegmentReaderTest < Test::Unit::TestCase
  include IndexReaderTest

  def setup()
    @dir = RAMDirectory.new()
    iw = IndexWriter.new(@dir, WhiteSpaceAnalyzer.new(), true, false)
    docs = IndexTestHelper.prepare_ir_test_docs()
    IndexTestHelper::IR_TEST_DOC_CNT.times do |i|
      iw << docs[i]
    end

    iw.optimize()
    iw.close()
    @ir = IndexReader.open(@dir, false)
  end

  def tear_down()
    @ir.close()
    @dir.close()
  end
end

