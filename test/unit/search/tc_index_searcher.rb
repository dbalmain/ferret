require File.dirname(__FILE__) + "/../../test_helper"

class IndexSearcherTest < Test::Unit::TestCase
  include Ferret::Document
  include Ferret::Search
  include Ferret::Store
  include Ferret::Analysis
  include Ferret::Index

  def setup()
    @dir = RAMDirectory.new()
    iw = IndexWriter.new(@dir, :analyzer => WhiteSpaceAnalyzer.new(), :create => true)
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

  def check_hits(query, expected, top=nil, total_hits=nil)
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

  def check_docs(query, options, expected=[])
    top_docs = @is.search(query, options)
    docs = top_docs.score_docs
    assert_equal(expected.length, docs.length)
    docs.length.times do |i|
      assert_equal(expected[i], docs[i].doc)
    end
  end

  def test_get_doc()
    assert_equal(18, @is.max_doc)
    assert_equal("20050930", @is.doc(0).values(:date))
    assert_equal("cat1/sub2/subsub2", @is.doc(4)[:cat])
  end

  def test_term_query
    tq = TermQuery.new(Term.new("field", "word2"))
    tq.boost = 100
    check_hits(tq, [1,4,8])
    #puts @is.explain(tq, 1)
    #puts @is.explain(tq, 4)
    #puts @is.explain(tq, 8)

    tq = TermQuery.new(Term.new("field", "2342"))
    check_hits(tq, [])

    tq = TermQuery.new(Term.new("field", ""))
    check_hits(tq, [])

    tq = TermQuery.new(Term.new("field", "word1"))
    top_docs = @is.search(tq)
    assert_equal(@documents.size, top_docs.total_hits)
    assert_equal(10, top_docs.score_docs.size)
    top_docs = @is.search(tq, {:num_docs => 20})
    assert_equal(@documents.size, top_docs.score_docs.size)
  end


  def test_first_doc
    tq = TermQuery.new(Term.new("field", "word1"))
    tq.boost = 100
    top_docs = @is.search(tq, {:num_docs => 100})
    expected = []
    top_docs.score_docs.each do |sd|
      expected << sd.doc
    end

    assert_raise(ArgumentError) { @is.search(tq, {:first_doc => -1}) }
    assert_raise(ArgumentError) { @is.search(tq, {:num_docs => 0}) }
    assert_raise(ArgumentError) { @is.search(tq, {:num_docs => -1}) }

    check_docs(tq, {:num_docs => 8, :first_doc => 0}, expected[0,8])
    check_docs(tq, {:num_docs => 3, :first_doc => 1}, expected[1,3])
    check_docs(tq, {:num_docs => 6, :first_doc => 2}, expected[2,6])
    check_docs(tq, {:num_docs => 2, :first_doc => expected.length}, [])
    check_docs(tq, {:num_docs => 2, :first_doc => expected.length + 100}, [])
  end

  def test_boolean_query
    bq = BooleanQuery.new()
    tq1 = TermQuery.new(Term.new("field", "word1"))
    tq2 = TermQuery.new(Term.new("field", "word3"))
    bq.add_query(tq1, BooleanClause::Occur::MUST)
    bq.add_query(tq2, BooleanClause::Occur::MUST)
    check_hits(bq, [2,3,6,8,11,14], 14)

    tq3 = TermQuery.new(Term.new("field", "word2"))
    bq.add_query(tq3, BooleanClause::Occur::SHOULD)
    check_hits(bq, [2,3,6,8,11,14], 8)

    bq = BooleanQuery.new()
    bq.add_query(tq2, BooleanClause::Occur::MUST)
    bq.add_query(tq3, BooleanClause::Occur::MUST_NOT)
    check_hits(bq, [2,3,6,11,14])

    bq = BooleanQuery.new()
    bq.add_query(tq2, BooleanClause::Occur::MUST_NOT)
    check_hits(bq, [])

    bq = BooleanQuery.new()
    bq.add_query(tq2, BooleanClause::Occur::SHOULD)
    bq.add_query(tq3, BooleanClause::Occur::SHOULD)
    check_hits(bq, [1,2,3,4,6,8,11,14])
  end

  def test_phrase_query()
    pq = PhraseQuery.new()
    t1 = Term.new("field", "quick")
    t2 = Term.new("field", "brown")
    t3 = Term.new("field", "fox")
    pq << t1 << t2 << t3
    check_hits(pq, [1])

    pq = PhraseQuery.new()
    pq << t1
    pq.add(t3, 2)
    check_hits(pq, [1,11,14])

    pq.slop = 1
    check_hits(pq, [1,11,14,16])

    pq.slop = 4
    check_hits(pq, [1,11,14,16,17])
  end

  def test_range_query()
    rq = RangeQuery.new("date", "20051006", "20051010", true, true)
    check_hits(rq, [6,7,8,9,10])

    rq = RangeQuery.new("date", "20051006", "20051010", false, true)
    check_hits(rq, [7,8,9,10])

    rq = RangeQuery.new("date", "20051006", "20051010", true, false)
    check_hits(rq, [6,7,8,9])

    rq = RangeQuery.new("date", "20051006", "20051010", false, false)
    check_hits(rq, [7,8,9])

    rq = RangeQuery.new("date", nil, "20051003", false, true)
    check_hits(rq, [0,1,2,3])

    rq = RangeQuery.new("date", nil, "20051003", false, false)
    check_hits(rq, [0,1,2])

    rq = RangeQuery.new_less("date", "20051003", true)
    check_hits(rq, [0,1,2,3])

    rq = RangeQuery.new_less("date", "20051003", false)
    check_hits(rq, [0,1,2])

    rq = RangeQuery.new("date", "20051014", nil, true, false)
    check_hits(rq, [14,15,16,17])

    rq = RangeQuery.new("date", "20051014", nil, false, false)
    check_hits(rq, [15,16,17])

    rq = RangeQuery.new_more("date", "20051014", true)
    check_hits(rq, [14,15,16,17])

    rq = RangeQuery.new_more("date", "20051014", false)
    check_hits(rq, [15,16,17])
  end

  def test_prefix_query()
    t = Term.new("cat", "cat1")
    pq = PrefixQuery.new(t)
    check_hits(pq, [0, 1, 2, 3, 4, 13, 14, 15, 16, 17])

    t.text = "cat1/sub2"
    pq = PrefixQuery.new(t)
    check_hits(pq, [3, 4, 13, 15])
  end

  def test_wildcard_query()
    t = Term.new("cat", "cat1*")
    wq = WildcardQuery.new(t)
    check_hits(wq, [0, 1, 2, 3, 4, 13, 14, 15, 16, 17])

    t.text = "cat1*/su??ub2"
    wq = WildcardQuery.new(t)
    check_hits(wq, [4, 16])
  end

  def test_multi_phrase_query()
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
    check_hits(mpq, [1, 8, 11, 14])

    mpq.slop = 4
    check_hits(mpq, [1, 8, 11, 14, 16, 17])
  end
end
