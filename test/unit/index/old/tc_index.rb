require File.dirname(__FILE__) + "/../../test_helper"
require File.dirname(__FILE__) + "/th_doc_helper"

include Ferret::Index
include Ferret::Store

class IndexTest < Test::Unit::TestCase
  def setup
    @dir = RAMDirectory.new
  end

  def teardown
    @dir.close
  end

  def test_doc_count
    index = Index.new(@dir)
    docs = DocHelper.prepare_book_list
    docs.each {|doc| index << doc }
    assert_equal(37, index.size())
  end

  def test_lookup
    index = Index.new(@dir)
    docs = DocHelper.prepare_book_list
    docs.each {|doc| index << doc }
    docs = index.get_docs_with_term(Term.new("author", "carey"))
    assert_equal("Oscar and Lucinda", docs[0]["title"])
    assert_equal("True History of the Kelly Gang", docs[1]["title"])
  end

  def test_read_write
    index = Index.new(@dir)
    docs = DocHelper.prepare_book_list
    docs.each {|doc| index << doc }
    index.close
    index = Index.new(@dir)
    docs = index.get_docs_with_term(Term.new("author", "carey"))
    assert_equal("Oscar and Lucinda", docs[0]["title"])
    assert_equal("True History of the Kelly Gang", docs[1]["title"])
  end
end
