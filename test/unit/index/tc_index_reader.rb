require File.dirname(__FILE__) + "/../../test_helper"

module IndexReaderCommon

  include Ferret::Index
  include Ferret::Analysis

  def test_index_reader

    do_test_term_doc_enum()
    
    do_test_term_vectors()

    do_test_changing_field()

    do_test_get_doc()

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
    tv = @ir.get_term_vector(0, "changing_field")
    assert(tv.nil?)

    tv = @ir.get_term_vector(10, "changing_field")
    assert(tv.positions.nil?)
    assert(tv.offsets.nil?)

    tv = @ir.get_term_vector(17, "changing_field")
    assert(tv.positions)
    assert(tv.offsets.nil?)

    tv = @ir.get_term_vector(19, "changing_field")
    assert(tv.positions.nil?)
    assert(tv.offsets)

    tv = @ir.get_term_vector(20, "changing_field")
    assert(tv.positions)
    assert(tv.offsets)

    tv = @ir.get_term_vector(21, "changing_field")
    assert(tv.nil?)
  end

  def do_test_get_doc()
    doc = @ir.get_document(3)
    assert_equal(4, doc.field_count)

    df = doc.field("author")
    assert_equal("author", df.name)
    assert_equal("Leo Tolstoy", df.data)
    assert_equal(df.boost, 1.0)
    assert_equal(true, df.stored?)
    assert_equal(false, df.compressed?)
    assert_equal(true, df.indexed?)
    assert_equal(true, df.tokenized?)
    assert_equal(true, df.store_term_vector?)
    assert_equal(true, df.store_positions?)
    assert_equal(false, df.store_offsets?)
    assert_equal(false, df.binary?)
    
    df = doc.field("body")
    assert_equal("body", df.name)
    assert_equal("word3 word4 word1 word2 word1 word3 word4 word1 word3 word3", df.data)
    assert_equal(df.boost, 1.0)
    assert_equal(true, df.stored?)
    assert_equal(false, df.compressed?)
    assert_equal(true, df.indexed?)
    assert_equal(true, df.tokenized?)
    assert_equal(true, df.store_term_vector?)
    assert_equal(true, df.store_positions?)
    assert_equal(true, df.store_offsets?)
    assert_equal(false, df.binary?)
    
    df = doc.field("title")
    assert_equal("title", df.name)
    assert_equal("War And Peace", df.data)
    assert_equal(df.boost, 1.0)
    assert_equal(true, df.stored?)
    assert_equal(false, df.compressed?)
    assert_equal(true, df.indexed?)
    assert_equal(false, df.tokenized?)
    assert_equal(true, df.store_term_vector?)
    assert_equal(false, df.store_positions?)
    assert_equal(true, df.store_offsets?)
    assert_equal(false, df.binary?)

    df = doc.field("year")
    assert_equal("year", df.name)
    assert_equal("1865", df.data)
    assert_equal(df.boost, 1.0)
    assert_equal(true, df.stored?)
    assert_equal(false, df.compressed?)
    assert_equal(false, df.indexed?)
    assert_equal(false, df.tokenized?)
    assert_equal(false, df.store_term_vector?)
    assert_equal(false, df.store_positions?)
    assert_equal(false, df.store_offsets?)
    assert_equal(false, df.binary?)


    df = doc.field("text")
    assert(df.nil?) # "text" is not stored
  end

  def test_ir_norms()
    @ir.set_norm(3, "title", 1)
    @ir.set_norm(3, "body", 12)
    @ir.set_norm(3, "author", 145)
    @ir.set_norm(3, "year", 31)
    @ir.set_norm(3, "text", 202)
    @ir.set_norm(25, "text", 20)
    @ir.set_norm(50, "text", 200)
    @ir.set_norm(63, "text", 155)

    norms = @ir.get_norms("text")

    assert_equal(202, norms[3])
    assert_equal(20, norms[25])
    assert_equal(200, norms[50])
    assert_equal(155, norms[63])

    norms = @ir.get_norms("title")
    assert_equal(1, norms[3])

    norms = @ir.get_norms("body")
    assert_equal(12, norms[3])

    norms = @ir.get_norms("author")
    assert_equal(145, norms[3])

    norms = @ir.get_norms("year")
    # TODO: this returns two possible results depending on whether it is 
    # a multi reader or a segment reader. If it is a multi reader it will
    # always return an empty set of norms, otherwise it will return nil. 
    # I'm not sure what to do here just yet or if this is even an issue.
    #assert(norms.nil?) 

    norms = " " * 164
    @ir.get_norms_into("text", norms, 100)
    assert_equal(202, norms[103])
    assert_equal(20, norms[125])
    assert_equal(200, norms[150])
    assert_equal(155, norms[163])

    @ir.commit()

    iw = IndexWriter.new(@dir, :analyzer => WhiteSpaceAnalyzer.new())
    iw.optimize()
    iw.close()

    ir2 = IndexReader.open(@dir, false)

    norms = " " * 164
    ir2.get_norms_into("text", norms, 100)
    assert_equal(202, norms[103])
    assert_equal(20, norms[125])
    assert_equal(200, norms[150])
    assert_equal(155, norms[163])
    ir2.close()
  end

  def test_ir_delete()
    doc_count = IndexTestHelper::IR_TEST_DOC_CNT
    assert_equal(false, @ir.has_deletions?())
    assert_equal(doc_count, @ir.max_doc())
    assert_equal(doc_count, @ir.num_docs())
    assert_equal(false, @ir.deleted?(10))

    @ir.delete(10)
    assert_equal(true, @ir.has_deletions?())
    assert_equal(doc_count, @ir.max_doc())
    assert_equal(doc_count - 1, @ir.num_docs())
    assert_equal(true, @ir.deleted?(10))

    @ir.delete(10)
    assert_equal(true, @ir.has_deletions?())
    assert_equal(doc_count, @ir.max_doc())
    assert_equal(doc_count - 1, @ir.num_docs())
    assert_equal(true, @ir.deleted?(10))

    @ir.delete(doc_count - 1)
    assert_equal(true, @ir.has_deletions?())
    assert_equal(doc_count, @ir.max_doc())
    assert_equal(doc_count - 2, @ir.num_docs())
    assert_equal(true, @ir.deleted?(doc_count - 1))

    @ir.delete(doc_count - 2)
    assert_equal(true, @ir.has_deletions?())
    assert_equal(doc_count, @ir.max_doc())
    assert_equal(doc_count - 3, @ir.num_docs())
    assert_equal(true, @ir.deleted?(doc_count - 2))

    @ir.undelete_all()
    assert_equal(false, @ir.has_deletions?())
    assert_equal(doc_count, @ir.max_doc())
    assert_equal(doc_count, @ir.num_docs())
    assert_equal(false, @ir.deleted?(10))
    assert_equal(false, @ir.deleted?(doc_count - 2))
    assert_equal(false, @ir.deleted?(doc_count - 1))

    @ir.delete(10)
    @ir.delete(20)
    @ir.delete(30)
    @ir.delete(40)
    @ir.delete(50)
    @ir.delete(doc_count - 1)
    assert_equal(true, @ir.has_deletions?())
    assert_equal(doc_count, @ir.max_doc())
    assert_equal(doc_count - 6, @ir.num_docs())

    @ir.commit()

    ir2 = IndexReader.open(@dir, false)

    assert_equal(true, ir2.has_deletions?())
    assert_equal(doc_count, ir2.max_doc())
    assert_equal(doc_count - 6, ir2.num_docs())
    assert_equal(true, ir2.deleted?(10))
    assert_equal(true, ir2.deleted?(20))
    assert_equal(true, ir2.deleted?(30))
    assert_equal(true, ir2.deleted?(40))
    assert_equal(true, ir2.deleted?(50))
    assert_equal(true, ir2.deleted?(doc_count - 1))

    ir2.undelete_all()
    assert_equal(false, ir2.has_deletions?())
    assert_equal(doc_count, ir2.max_doc())
    assert_equal(doc_count, ir2.num_docs())
    assert_equal(false, ir2.deleted?(10))
    assert_equal(false, ir2.deleted?(20))
    assert_equal(false, ir2.deleted?(30))
    assert_equal(false, ir2.deleted?(40))
    assert_equal(false, ir2.deleted?(50))
    assert_equal(false, ir2.deleted?(doc_count - 1))

    ir2.delete(10)
    ir2.delete(20)
    ir2.delete(30)
    ir2.delete(40)
    ir2.delete(50)
    ir2.delete(doc_count - 1)

    ir2.commit()

    iw = IndexWriter.new(@dir, :analyzer => WhiteSpaceAnalyzer.new())
    iw.optimize()
    iw.close()

    ir3 = IndexReader.open(@dir, false)

    assert(!ir3.has_deletions?())
    assert_equal(doc_count - 6, ir3.max_doc())
    assert_equal(doc_count - 6, ir3.num_docs())

    ir3.close()
  end
 
end

class SegmentReaderTest < Test::Unit::TestCase
  include IndexReaderCommon

  def setup()
    @dir = Ferret::Store::RAMDirectory.new()
    iw = IndexWriter.new(@dir, :analyzer => WhiteSpaceAnalyzer.new(), :create => true)
    docs = IndexTestHelper.prepare_ir_test_docs()
    IndexTestHelper::IR_TEST_DOC_CNT.times do |i|
      iw << docs[i]
    end

    # we must optimize here so that SegmentReader is used.
    iw.optimize()
    iw.close()
    @ir = IndexReader.open(@dir, false)
  end

  def tear_down()
    @ir.close()
    @dir.close()
  end
end

class MultiReaderTest < Test::Unit::TestCase
  include IndexReaderCommon

  def setup()
    @dir = Ferret::Store::RAMDirectory.new()
    iw = IndexWriter.new(@dir, :analyzer => WhiteSpaceAnalyzer.new(), :create => true)
    docs = IndexTestHelper.prepare_ir_test_docs()
    IndexTestHelper::IR_TEST_DOC_CNT.times do |i|
      iw << docs[i]
    end

    # we mustn't optimize here so that MultiReader is used.
    # iw.optimize()
    iw.close()
    @ir = IndexReader.open(@dir, false)
  end

  def tear_down()
    @ir.close()
    @dir.close()
  end
end

class IndexReaderTest < Test::Unit::TestCase
  include Ferret::Index
  include Ferret::Analysis
  include Ferret::Document

  def setup()
    @dir = Ferret::Store::RAMDirectory.new()
  end

  def tear_down()
    @dir.close()
  end

  def test_ir_multivalue_fields()
    iw = IndexWriter.new(@dir, :analyzer => WhiteSpaceAnalyzer.new(), :create => true)
    doc = Document.new()
    doc << Field.new("tag", "Ruby", Field::Store::YES, Field::Index::NO, Field::TermVector::NO)
    doc << Field.new("tag", "C", Field::Store::YES, Field::Index::UNTOKENIZED, Field::TermVector::NO)
    doc << Field.new("body", "this is the body Document Field", Field::Store::YES, Field::Index::UNTOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    doc << Field.new("tag", "Lucene", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS)
    doc << Field.new("tag", "Ferret", Field::Store::YES, Field::Index::UNTOKENIZED, Field::TermVector::WITH_OFFSETS)
    doc << Field.new("title", "this is the title DocField", Field::Store::YES, Field::Index::UNTOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    doc << Field.new("author", "this is the author field", Field::Store::YES, Field::Index::UNTOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)

    fis = FieldInfos.new()
    fis << doc
    assert_equal(4, fis.size)

    fi = fis["tag"]
    assert_equal(true, fi.indexed?)
    assert_equal(true, fi.store_term_vector?)
    assert_equal(true, fi.store_positions?)
    assert_equal(true, fi.store_offsets?)

    iw << doc
    iw.close()

    ir = IndexReader.open(@dir, false)

    doc = ir.get_document(0)
    assert_equal(4, doc.field_count)
    assert_equal(7, doc.entry_count)
    entries = doc.fields("tag")
    assert_equal(4, entries.size)
    assert_equal("Ruby", entries[0].data)
    assert_equal("C", entries[1].data)
    assert_equal("Lucene", entries[2].data)
    assert_equal("Ferret", entries[3].data)

    doc.remove_field("tag")
    assert_equal(4, doc.field_count)
    assert_equal(6, doc.entry_count)
    assert_equal("C", doc.field("tag").data)

    doc.remove_fields("tag")
    assert_equal(3, doc.field_count)
    assert_equal(3, doc.entry_count)

    ir.delete(0)
    ir.close()

    iw = IndexWriter.new(@dir, :analyzer => WhiteSpaceAnalyzer.new())
    iw << doc
    iw.optimize()
    iw.close()
    doc = nil

    ir = IndexReader.open(@dir, false)
    doc = ir.get_document(0)
    assert_equal(3, doc.field_count)
    assert_equal(3, doc.entry_count)

    ir.close()
  end

  def t(start_offset, end_offset)
    TermVectorOffsetInfo.new(start_offset, end_offset)
  end

  def do_test_term_vectors(ir)
    tv = ir.get_term_vector(3, "body")

    assert_equal("body", tv.field)
    assert_equal(["word1", "word2", "word3", "word4"], tv.terms)
    assert_equal([3, 1, 4, 2], tv.term_frequencies)
    assert_equal([[2, 4, 7], [3], [0, 5, 8, 9], [1,6]], tv.positions)
    assert_equal([[t(12,17), t(24,29), t(42,47)],
                  [t(18,23)],
                  [t(0,5), t(30,35), t(48,53), t(54,59)],
                  [t(6,11), t(36,41)]], tv.offsets)
    tv = nil

    tvs = ir.get_term_vectors(3)
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

  def test_ir_read_while_optimizing()
    iw = IndexWriter.new(@dir, :analyzer => WhiteSpaceAnalyzer.new(), :create => true)
    docs = IndexTestHelper.prepare_ir_test_docs()
    IndexTestHelper::IR_TEST_DOC_CNT.times do |i|
      iw << docs[i]
    end
    iw.close()

    ir = IndexReader.open(@dir, false)
    do_test_term_vectors(ir)
    
    iw = IndexWriter.new(@dir, :analyzer => WhiteSpaceAnalyzer.new())
    iw.optimize()
    iw.close()

    do_test_term_vectors(ir)

    ir.close()
  end

  def test_ir_read_while_optimizing_on_disk()
    dpath = File.join(File.dirname(__FILE__),
                       '../../temp/fsdir')
    fs_dir = Ferret::Store::FSDirectory.new(dpath, true)

    iw = IndexWriter.new(fs_dir, :analyzer => WhiteSpaceAnalyzer.new(), :create => true)
    docs = IndexTestHelper.prepare_ir_test_docs()
    IndexTestHelper::IR_TEST_DOC_CNT.times do |i|
      iw << docs[i]
    end
    iw.close()

    ir = IndexReader.open(fs_dir, false)
    do_test_term_vectors(ir)
    
    iw = IndexWriter.new(fs_dir, :analyzer => WhiteSpaceAnalyzer.new())
    iw.optimize()
    iw.close()

    do_test_term_vectors(ir)

    ir.close()
    fs_dir.close()
  end

  def test_latest()
    dpath = File.join(File.dirname(__FILE__),
                       '../../temp/fsdir')
    fs_dir = Ferret::Store::FSDirectory.new(dpath, true)

    iw = IndexWriter.new(fs_dir, :analyzer => WhiteSpaceAnalyzer.new(), :create => true)
    doc = Document.new
    doc << Field.new("field", "content", Field::Store::YES, Field::Index::TOKENIZED)
    iw << doc
    iw.close()

    ir = IndexReader.open(fs_dir, false)
    assert(ir.latest?)

    iw = IndexWriter.new(fs_dir, :analyzer => WhiteSpaceAnalyzer.new())
    doc = Document.new
    doc << Field.new("field", "content2", Field::Store::YES, Field::Index::TOKENIZED)
    iw << doc
    iw.close()

    assert(!ir.latest?)

    ir.close()
    ir = IndexReader.open(fs_dir, false)
    assert(ir.latest?)
    ir.close()
  end
end

