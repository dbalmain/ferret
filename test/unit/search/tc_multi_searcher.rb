require File.dirname(__FILE__) + "/../../test_helper"
require File.join(File.dirname(__FILE__), "tc_index_searcher.rb")

# make sure a MultiSearcher searching only one index
# passes all the IndexSearcher tests
class SimpleMultiSearcherTest < IndexSearcherTest
  alias :old_setup :setup 
  def setup()
    old_setup
    @multi = MultiSearcher.new([IndexSearcher.new(@dir)])
  end
end


# checks query results of a multisearcher searching two indexes
# against those of a single indexsearcher searching the same
# set of documents
class MultiSearcherTest < Test::Unit::TestCase
  include Ferret::Document
  include Ferret::Search
  include Ferret::Store
  include Ferret::Analysis
  include Ferret::Index

  def prepare_search_docs(data)
    docs = []
    data.each_with_index do |fields, i|
      doc = Document.new()
      fields.each_pair do |field, text|
        doc << Field.new(field, text, Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::NO, false)
      end
      docs << doc
    end
    return docs
  end

  def prepare_documents
    @documents = prepare_search_docs([
      {"date" => "20050930", "field" => "word1",
        "cat" => "cat1/"},
      {"date" => "20051001", "field" => "word1 word2 the quick brown fox",
        "cat" => "cat1/sub1"},
      {"date" => "20051002", "field" => "word1 word3",
        "cat" => "cat1/sub1/subsub1"},
      {"date" => "20051003", "field" => "word1 word3",
        "cat" => "cat1/sub2"},
      {"date" => "20051004", "field" => "word1 word2",
        "cat" => "cat1/sub2/subsub2"},
      {"date" => "20051005", "field" => "word1",
        "cat" => "cat2/sub1"},
      {"date" => "20051006", "field" => "word1 word3",
        "cat" => "cat2/sub1"},
      {"date" => "20051007", "field" => "word1",
        "cat" => "cat2/sub1"},
      {"date" => "20051008", "field" => "word1 word2 word3 the fast brown fox",
        "cat" => "cat2/sub1"}
    ])
    @documents2 = prepare_search_docs([
      {"date" => "20051009", "field" => "word1",
        "cat" => "cat3/sub1"},
      {"date" => "20051010", "field" => "word1",
        "cat" => "cat3/sub1"},
      {"date" => "20051011", "field" => "word1 word3 the quick red fox",
        "cat" => "cat3/sub1"},
      {"date" => "20051012", "field" => "word1",
        "cat" => "cat3/sub1"},
      {"date" => "20051013", "field" => "word1",
        "cat" => "cat1/sub2"},
      {"date" => "20051014", "field" => "word1 word3 the quick hairy fox",
        "cat" => "cat1/sub1"},
      {"date" => "20051015", "field" => "word1",
        "cat" => "cat1/sub2/subsub1"},
      {"date" => "20051016",
        "field" => "word1 the quick fox is brown and hairy and a little red",
        "cat" => "cat1/sub1/subsub2"},
      {"date" => "20051017", "field" => "word1 the brown fox is quick and red",
        "cat" => "cat1/"}
    ]) 
  end

  def setup()
    prepare_documents
    # create MultiSearcher from two seperate searchers
    dir1 = RAMDirectory.new()
    iw1 = IndexWriter.new(dir1, :analyzer => WhiteSpaceAnalyzer.new(), :create => true)
    @documents.each { |doc| iw1 << doc }
    iw1.close()
    
    dir2 = RAMDirectory.new()
    iw2 = IndexWriter.new(dir2, :analyzer => WhiteSpaceAnalyzer.new(), :create => true)
    @documents2.each { |doc| iw2 << doc }
    iw2.close()
    @multi = Ferret::Search::MultiSearcher.new([IndexSearcher.new(dir1), IndexSearcher.new(dir2)])

    # create single searcher
    dir = RAMDirectory.new
    iw = IndexWriter.new(dir, :analyzer => WhiteSpaceAnalyzer.new(), :create => true)
    @documents.each { |doc| iw << doc }
    @documents2.each { |doc| iw << doc }
    iw.close
    @single = IndexSearcher.new(dir)

    @query_parser = Ferret::QueryParser.new(['date', 'field', 'cat'], :analyzer => WhiteSpaceAnalyzer.new())
  end

  def tear_down()
    @multi.close
    @single.close
  end

  def check_hits(query, debug_field=nil)
    query = @query_parser.parse(query) if (query.is_a? String)
    multi_docs = @multi.search(query)
    single_docs = @single.search(query)
    IndexTestHelper.explain(query, @single, debug_field) if debug_field
    IndexTestHelper.explain(query, @multi, debug_field) if debug_field
    assert_equal(single_docs.score_docs.size, multi_docs.score_docs.size, 'hit count')
    assert_equal(single_docs.total_hits, multi_docs.total_hits, 'hit count')
    
    multi_docs.score_docs.each_with_index { |sd, id|
      assert_equal single_docs.score_docs[id].doc, sd.doc
      assert_equal single_docs.score_docs[id].score, sd.score
    }
  end

  def test_term_query
    tq = TermQuery.new(Term.new("field", "word2"));
    tq.boost = 100
    check_hits(tq)

    tq = TermQuery.new(Term.new("field", "2342"));
    check_hits(tq)

    tq = TermQuery.new(Term.new("field", ""));
    check_hits(tq)

    tq = TermQuery.new(Term.new("field", "word1"));
    check_hits(tq)
  end


  def test_boolean_query
    bq = BooleanQuery.new()
    tq1 = TermQuery.new(Term.new("field", "word1"))
    tq2 = TermQuery.new(Term.new("field", "word3"))
    bq.add_query(tq1, BooleanClause::Occur::MUST)
    bq.add_query(tq2, BooleanClause::Occur::MUST)
    check_hits(bq)

    tq3 = TermQuery.new(Term.new("field", "word2"))
    bq.add_query(tq3, BooleanClause::Occur::SHOULD)
    check_hits(bq)

    bq = BooleanQuery.new()
    bq.add_query(tq2, BooleanClause::Occur::MUST)
    bq.add_query(tq3, BooleanClause::Occur::MUST_NOT)
    check_hits(bq)

    bq = BooleanQuery.new()
    bq.add_query(tq2, BooleanClause::Occur::MUST_NOT)
    check_hits(bq)

    bq = BooleanQuery.new()
    bq.add_query(tq2, BooleanClause::Occur::SHOULD)
    bq.add_query(tq3, BooleanClause::Occur::SHOULD)
    check_hits(bq)
  end

  def test_phrase_query()
    pq = PhraseQuery.new()
    t1 = Term.new("field", "quick")
    t2 = Term.new("field", "brown")
    t3 = Term.new("field", "fox")
    pq << t1 << t2 << t3
    check_hits(pq)

    pq = PhraseQuery.new()
    pq << t1
    pq.add(t3, 2)
    check_hits(pq)

    pq.slop = 1
    check_hits(pq)

    pq.slop = 4
    check_hits(pq)
  end

  def test_range_query()
    rq = RangeQuery.new("date", "20051006", "20051010", true, true)
    check_hits(rq)

    rq = RangeQuery.new("date", "20051006", "20051010", false, true)
    check_hits(rq)

    rq = RangeQuery.new("date", "20051006", "20051010", true, false)
    check_hits(rq)

    rq = RangeQuery.new("date", "20051006", "20051010", false, false)
    check_hits(rq)

    rq = RangeQuery.new("date", nil, "20051003", false, true)
    check_hits(rq)

    rq = RangeQuery.new("date", nil, "20051003", false, false)
    check_hits(rq)

    rq = RangeQuery.new_less("date", "20051003", true)
    check_hits(rq)

    rq = RangeQuery.new_less("date", "20051003", false)
    check_hits(rq)

    rq = RangeQuery.new("date", "20051014", nil, true, false)
    check_hits(rq)

    rq = RangeQuery.new("date", "20051014", nil, false, false)
    check_hits(rq)

    rq = RangeQuery.new_more("date", "20051014", true)
    check_hits(rq)

    rq = RangeQuery.new_more("date", "20051014", false)
    check_hits(rq)
  end

  def test_prefix_query()
    t = Term.new("cat", "cat1")
    pq = PrefixQuery.new(t)
    check_hits(pq)

    t.text = "cat1/sub2"
    pq = PrefixQuery.new(t)
    check_hits(pq)
  end

  def test_wildcard_query()
    t = Term.new("cat", "cat1*")
    wq = WildcardQuery.new(t)
    check_hits(wq)

    t.text = "cat1*/su??ub2"
    wq = WildcardQuery.new(t)
    check_hits(wq)
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
    check_hits(mpq)

    mpq.slop = 4
    check_hits(mpq)
  end
end
