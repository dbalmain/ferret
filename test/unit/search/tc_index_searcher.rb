require File.dirname(__FILE__) + "/../../test_helper"

include Ferret::Search
include Ferret::Store
include Ferret::Analysis

class IndexSearcherTest < Test::Unit::TestCase
  def setup()
    @dir = RAMDirectory.new()
  end

  def tear_down()
    @dir.close()
  end

  def test_boolean_query
    iw = IndexWriter.new(@dir, WhiteSpaceAnalyzer.new(), true, false)
    docs = IndexTestHelper.prepare_book_list()
    docs.each { |doc| iw << doc }
    iw.close()
    term_query = TermQuery.new(Term.new("title", "Life"));
    term_query.boost = 100
    is = IndexSearcher.new(@dir)
    top_docs = is.search(term_query)
    assert_equal(2, top_docs.total_hits)
    assert_equal(35, top_docs.score_docs[0].doc)
    #puts is.explain(term_query, 35).to_html
    assert_equal(15, top_docs.score_docs[1].doc)
    #puts is.explain(term_query, 15)
  end

  def test_term_query
    iw = IndexWriter.new(@dir, WhiteSpaceAnalyzer.new(), true, false)
    docs = IndexTestHelper.prepare_book_list()
    docs.each { |doc| iw << doc }
    iw.close()
    term_query = TermQuery.new(Term.new("title", "Life"));
    term_query.boost = 100
    is = IndexSearcher.new(@dir)
    top_docs = is.search(term_query)
    assert_equal(2, top_docs.total_hits)
    assert_equal(35, top_docs.score_docs[0].doc)
    #puts is.explain(term_query, 35).to_html
    assert_equal(15, top_docs.score_docs[1].doc)
    #puts is.explain(term_query, 15)
  end
end
