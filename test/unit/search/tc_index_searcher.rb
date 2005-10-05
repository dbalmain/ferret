require File.dirname(__FILE__) + "/../../test_helper"

include Ferret::Search
include Ferret::Store
include Ferret::Analysis

class IndexSearcherTest < Test::Unit::TestCase
  def setup()
    @dir = RAMDirectory.new()
    iw = IndexWriter.new(@dir, WhiteSpaceAnalyzer.new(), true, false)
    @documents = IndexTestHelper.prepare_search_docs()
    @documents.each { |doc| iw << doc }
    iw.close()
  end

  def tear_down()
    @dir.close()
  end

  def get_docs(score_docs)
    docs = []
    score_docs.each do |score_doc|
      docs << score_doc.doc
    end
    docs
  end

  def test_term_query
    term_query = TermQuery.new(Term.new("field", "word2"));
    term_query.boost = 100
    is = IndexSearcher.new(@dir)
    top_docs = is.search(term_query)
    #puts top_docs.score_docs
    assert_equal(3, top_docs.total_hits)

    docs = get_docs(top_docs.score_docs)
    assert(docs.include?(1))
    assert(docs.include?(4))
    assert(docs.include?(8))
    #puts is.explain(term_query, 1).to_html
    #puts is.explain(term_query, 4)

    term_query = TermQuery.new(Term.new("field", "word1"));
    top_docs = is.search(term_query)
    #puts top_docs.score_docs
    assert_equal(@documents.size, top_docs.total_hits)
    assert_equal(10, top_docs.score_docs.size)
    top_docs = is.search(term_query, nil, 20)
    assert_equal(@documents.size, top_docs.score_docs.size)
  end

  def test_boolean_query
    boolean_query = BooleanQuery.new()
    term_query1 = TermQuery.new(Term.new("field", "word1"))
    term_query2 = TermQuery.new(Term.new("field", "word3"))
    boolean_query.add_query(term_query1, BooleanClause::Occur::MUST)
    boolean_query.add_query(term_query2, BooleanClause::Occur::MUST)
    is = IndexSearcher.new(@dir)
    top_docs = is.search(boolean_query)
    assert_equal(6, top_docs.total_hits)
    assert(top_docs.score_docs[0].doc != 8)

    top_docs.score_docs.each do |score_doc|
      assert_equal(score_doc.score, is.explain(boolean_query, score_doc.doc).value)
    end
    docs = get_docs(top_docs.score_docs)
    assert(docs.include?(2))
    assert(docs.include?(3))
    assert(docs.include?(6))
    assert(docs.include?(8))
    assert(docs.include?(11))
    assert(docs.include?(14))

    term_query3 = TermQuery.new(Term.new("field", "word2"))
    boolean_query.add_query(term_query3, BooleanClause::Occur::SHOULD)
    top_docs = is.search(boolean_query)
    assert_equal(6, top_docs.total_hits)
    assert(top_docs.score_docs[0].doc == 8)

    top_docs.score_docs.each do |score_doc|
      assert_equal(score_doc.score, is.explain(boolean_query, score_doc.doc).value)
    end
    docs = get_docs(top_docs.score_docs)
    assert(docs.include?(2))
    assert(docs.include?(3))
    assert(docs.include?(6))
    assert(docs.include?(8))
    assert(docs.include?(11))
    assert(docs.include?(14))
    
    boolean_query = BooleanQuery.new()
    boolean_query.add_query(term_query2, BooleanClause::Occur::MUST)
    boolean_query.add_query(term_query3, BooleanClause::Occur::MUST_NOT)
    top_docs = is.search(boolean_query)
    assert_equal(5, top_docs.total_hits)
    
    top_docs.score_docs.each do |score_doc|
      assert_equal(score_doc.score, is.explain(boolean_query, score_doc.doc).value)
    end
    docs = get_docs(top_docs.score_docs)
    assert(docs.include?(2))
    assert(docs.include?(3))
    assert(docs.include?(6))
    assert(docs.include?(11))
    assert(docs.include?(14))

    boolean_query = BooleanQuery.new()
    boolean_query.add_query(term_query2, BooleanClause::Occur::MUST_NOT)
    top_docs = is.search(boolean_query)
    assert_equal(0, top_docs.total_hits)

    boolean_query = BooleanQuery.new()
    boolean_query.add_query(term_query2, BooleanClause::Occur::SHOULD)
    boolean_query.add_query(term_query3, BooleanClause::Occur::SHOULD)
    top_docs = is.search(boolean_query)
    assert_equal(8, top_docs.total_hits)
    top_docs.score_docs.each do |score_doc|
      assert_equal(score_doc.score, is.explain(boolean_query, score_doc.doc).value)
    end
    puts top_docs.score_docs
    puts is.explain(boolean_query, 2)
    docs = get_docs(top_docs.score_docs)
    assert(docs.include?(1))
    assert(docs.include?(2))
    assert(docs.include?(3))
    assert(docs.include?(4))
    assert(docs.include?(6))
    assert(docs.include?(8))
    assert(docs.include?(11))
    assert(docs.include?(14))
  end
end
