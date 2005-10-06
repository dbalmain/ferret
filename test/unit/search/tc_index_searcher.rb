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
    @is = IndexSearcher.new(@dir)
  end

  def tear_down()
    @is.close
    @dir.close()
  end

  def get_docs(score_docs)
    docs = []
    score_docs.each do |score_doc|
      docs << score_doc.doc
    end
    docs
  end

  def do_test_top_docs(query, expected, top=nil, total_hits=nil)
    top_docs = @is.search(query)
    assert_equal(expected.length, top_docs.score_docs.size)
    assert_equal(top, top_docs.score_docs[0].doc) if top
    if total_hits
      assert_equal(total_hits, top_docs.total_hits)
    else
      assert_equal(expected.length, top_docs.total_hits)
    end
    top_docs.score_docs.each do |score_doc|
      assert(expected.include?(score_doc.doc))
      assert(score_doc.score =~ @is.explain(query, score_doc.doc).value)
    end
  end

  def test_term_query
    tq = TermQuery.new(Term.new("field", "word2"));
    tq.boost = 100
    do_test_top_docs(tq, [1,4,8])

    tq = TermQuery.new(Term.new("field", "word1"));
    top_docs = @is.search(tq)
    #puts top_docs.score_docs
    assert_equal(@documents.size, top_docs.total_hits)
    assert_equal(10, top_docs.score_docs.size)
    top_docs = @is.search(tq, nil, 20)
    assert_equal(@documents.size, top_docs.score_docs.size)
  end

  def test_boolean_query
    bq = BooleanQuery.new()
    tq1 = TermQuery.new(Term.new("field", "word1"))
    tq2 = TermQuery.new(Term.new("field", "word3"))
    bq.add_query(tq1, BooleanClause::Occur::MUST)
    bq.add_query(tq2, BooleanClause::Occur::MUST)
    do_test_top_docs(bq, [2,3,6,8,11,14], 14)

    tq3 = TermQuery.new(Term.new("field", "word2"))
    bq.add_query(tq3, BooleanClause::Occur::SHOULD)
    do_test_top_docs(bq, [2,3,6,8,11,14], 8)

    bq = BooleanQuery.new()
    bq.add_query(tq2, BooleanClause::Occur::MUST)
    bq.add_query(tq3, BooleanClause::Occur::MUST_NOT)
    do_test_top_docs(bq, [2,3,6,11,14])

    bq = BooleanQuery.new()
    bq.add_query(tq2, BooleanClause::Occur::MUST_NOT)
    do_test_top_docs(bq, [])

    bq = BooleanQuery.new()
    bq.add_query(tq2, BooleanClause::Occur::SHOULD)
    bq.add_query(tq3, BooleanClause::Occur::SHOULD)
    do_test_top_docs(bq, [1,2,3,4,6,8,11,14])
  end

  def test_phrase_query()
    pq = PhraseQuery.new()
    t1 = Term.new("field", "quick")
    t2 = Term.new("field", "brown")
    t3 = Term.new("field", "fox")
    pq << t1 << t2 << t3
    do_test_top_docs(pq, [1])

    pq.slop = 4
    do_test_top_docs(pq, [1,16,17])

    pq = PhraseQuery.new()
    pq << t1
    pq.add(t3, 2)
    do_test_top_docs(pq, [1,11,14])

    pq.slop = 1
    do_test_top_docs(pq, [1,11,14,16])

    pq.slop = 4
    do_test_top_docs(pq, [1,11,14,16,17])
  end
end
