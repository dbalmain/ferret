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
      assert(expected.include?(score_doc.doc),
             "#{score_doc.doc} was found unexpectedly")
      assert(score_doc.score =~ @is.explain(query, score_doc.doc).value, 
        "Scores(#{score_doc.score} != #{@is.explain(query, score_doc.doc).value})")
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
    top_docs = @is.search(tq, {:num_docs => 20})
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

  def test_range_query()
    t1 = Term.new("date", "20051006")
    t2 = Term.new("date", "20051009")
    rq = RangeQuery.new(t1, t2)
    do_test_top_docs(rq, [5,6,7,8])

    rq = RangeQuery.new(t1, t2, false)
    do_test_top_docs(rq, [6,7])

    t1.text = "20051003"
    rq = RangeQuery.new(nil, t1, false)
    do_test_top_docs(rq, [0,1])

    t1.text = "20051015"
    rq = RangeQuery.new(t1, nil, false)
    do_test_top_docs(rq, [15,16,17])
  end

  def test_prefix_query()
    t = Term.new("cat", "cat1")
    pq = PrefixQuery.new(t)
    do_test_top_docs(pq, [0, 1, 2, 3, 4, 13, 14, 15, 16, 17])

    t.text = "cat1/sub2"
    pq = PrefixQuery.new(t)
    do_test_top_docs(pq, [3, 4, 13, 15])
  end

  def test_wildcard_query()
    t = Term.new("cat", "cat1*")
    wq = WildcardQuery.new(t)
    do_test_top_docs(wq, [0, 1, 2, 3, 4, 13, 14, 15, 16, 17])

    t.text = "cat1*/su??ub2"
    wq = WildcardQuery.new(t)
    do_test_top_docs(wq, [4, 16])
  end

  def test_prefix_query()
    t11 = Term.new("field", "quick")
    t12 = Term.new("field", "fast")
    t21 = Term.new("field", "brown")
    t22 = Term.new("field", "red")
    t23 = Term.new("field", "hairy")
    t3 = Term.new("field", "fox")

    mpq = MultiPhraseQuery.new()
    mpq << [t11, t12]
    mpq << [t21, t22, t23]
    mpq << t3
    do_test_top_docs(mpq, [1, 8, 11, 14])

    mpq.slop = 4
    do_test_top_docs(mpq, [1, 8, 11, 14, 16, 17])
  end
end
