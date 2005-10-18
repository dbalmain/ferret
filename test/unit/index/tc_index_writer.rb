require File.dirname(__FILE__) + "/../../test_helper"


class IndexWriterTest < Test::Unit::TestCase
  include Ferret::Index
  include Ferret::Analysis

  def setup()
    @dir = Ferret::Store::RAMDirectory.new
  end

  def tear_down()
    @dir.close()
  end

  def test_initialize
    wlock = @dir.make_lock(IndexWriter::WRITE_LOCK_NAME)
    clock = @dir.make_lock(IndexWriter::COMMIT_LOCK_NAME)
    assert(! wlock.locked?)
    assert(! clock.locked?)
    iw = IndexWriter.new(@dir, :create => true)
    assert(@dir.exists?("segments"))
    assert(wlock.locked?)
    iw.close()
    assert(@dir.exists?("segments"))
    assert(! wlock.locked?)
    assert(! clock.locked?)
  end

  def test_add_document
    iw = IndexWriter.new(@dir, :analyzer => StandardAnalyzer.new(), :create => true)
    doc = IndexTestHelper.prepare_document()
    infos = FieldInfos.new
    infos << doc
    iw.add_document(doc)
    assert_equal(1, iw.doc_count)
    iw.close()
  end

  def test_add_documents
    iw = IndexWriter.new(@dir, :analyzer => StandardAnalyzer.new(), :create => true)
    # uncomment the following line to see logging
    #iw.info_stream = $stdout
    iw.merge_factor = 3
    iw.min_merge_docs = 3
    docs = IndexTestHelper.prepare_book_list()
    infos = FieldInfos.new
    infos << docs[0]
    docs.each_with_index do |doc, i|
      #puts "Index doc " + i.to_s
      iw.add_document(doc)
    end
    assert_equal(37, iw.doc_count)
    iw.close()
  end

end
