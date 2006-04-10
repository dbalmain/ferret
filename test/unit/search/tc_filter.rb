require File.dirname(__FILE__) + "/../../test_helper"


class FilterTest < Test::Unit::TestCase
  include Ferret::Document
  include Ferret::Search
  include Ferret::Analysis
  include Ferret::Index

  def add_doc(hash, writer)
    doc = Document.new()
    hash.each_pair do |field, text|
      doc << Field.new(field, text, Field::Store::NO, Field::Index::UNTOKENIZED)
    end
    writer << doc
  end

  def setup()
    @dir = Ferret::Store::RAMDirectory.new()
    iw = IndexWriter.new(@dir,
                         :analyzer => WhiteSpaceAnalyzer.new(),
                         :create => true)
    docs = [ 
      {"int"=>"0","date"=>"20040601","switch"=>"on"},
      {"int"=>"1","date"=>"20041001","switch"=>"off"},
      {"int"=>"2","date"=>"20051101","switch"=>"on"},
      {"int"=>"3","date"=>"20041201","switch"=>"off"},
      {"int"=>"4","date"=>"20051101","switch"=>"on"},
      {"int"=>"5","date"=>"20041201","switch"=>"off"},
      {"int"=>"6","date"=>"20050101","switch"=>"on"},
      {"int"=>"7","date"=>"20040701","switch"=>"off"},
      {"int"=>"8","date"=>"20050301","switch"=>"on"},
      {"int"=>"9","date"=>"20050401","switch"=>"off"}
    ]
    docs.each {|doc| add_doc(doc, iw)}
    iw.close
  end

  def tear_down()
    @dir.close()
  end

  def do_test_top_docs(is, query, expected, filter)
    top_docs = is.search(query, {:filter => filter})
    #puts top_docs
    assert_equal(expected.size, top_docs.score_docs.size)
    top_docs.total_hits.times do |i|
      assert_equal(expected[i], top_docs.score_docs[i].doc)
    end
  end

  def test_range_filter
    is = IndexSearcher.new(@dir)
    q = MatchAllQuery.new()
    rf = RangeFilter.new("int", "2", "6", true, true)
    do_test_top_docs(is, q, [2,3,4,5,6], rf)
    rf = RangeFilter.new("int", "2", "6", true, false)
    do_test_top_docs(is, q, [2,3,4,5], rf)
    rf = RangeFilter.new("int", "2", "6", false, true)
    do_test_top_docs(is, q, [3,4,5,6], rf)
    rf = RangeFilter.new("int", "2", "6", false, false)
    do_test_top_docs(is, q, [3,4,5], rf)
    rf = RangeFilter.new_more("int", "6")
    do_test_top_docs(is, q, [6,7,8,9], rf)
    rf = RangeFilter.new_more("int", "6", false)
    do_test_top_docs(is, q, [7,8,9], rf)
    rf = RangeFilter.new_less("int", "2")
    do_test_top_docs(is, q, [0,1,2], rf)
    rf = RangeFilter.new_less("int", "2", false)
    do_test_top_docs(is, q, [0,1], rf)
  end

  def test_range_filter_errors
    assert_raise(ArgumentError) {f = RangeFilter.new("", "asd", nil, false, true)}
    assert_raise(ArgumentError) {f = RangeFilter.new("", nil, "asd", true, false)}
    assert_raise(ArgumentError) {f = RangeFilter.new("", "ac", "ab", false, false)}
    assert_raise(ArgumentError) {f = RangeFilter.new("", nil, nil, false, false)}
  end

  def test_query_filter()
    is = IndexSearcher.new(@dir)
    q = MatchAllQuery.new()
    qf = QueryFilter.new(TermQuery.new(Term.new("switch", "on")))
    do_test_top_docs(is, q, [0,2,4,6,8], qf)
    # test again to test caching doesn't break it
    do_test_top_docs(is, q, [0,2,4,6,8], qf)
    qf = QueryFilter.new(TermQuery.new(Term.new("switch", "off")))
    do_test_top_docs(is, q, [1,3,5,7,9], qf)
  end

  def test_filtered_query
    is = IndexSearcher.new(@dir)
    q = MatchAllQuery.new()
    rf = RangeFilter.new("int", "2", "6", true, true)
    rq = FilteredQuery.new(q, rf)
    qf = QueryFilter.new(TermQuery.new(Term.new("switch", "on")))
    do_test_top_docs(is, rq, [2,4,6], qf)
    query = FilteredQuery.new(rq, qf)
    rf2 = RangeFilter.new_more("int", "3")
    do_test_top_docs(is, query, [4,6], rf2)
  end
  #def test_filtered_query
  #  is = IndexSearcher.new(@dir)
  #  q = MatchAllQuery.new()
  #  rf = RangeFilter.new("int", "2", "6", true, true)
  #  rq = FilteredQuery.new(q, rf)
  #  qf = QueryFilter.new(TermQuery.new(Term.new("switch", "on")))
  #  do_test_top_docs(is, rq, [2,4,6], qf)
  #  query = FilteredQuery.new(rq, qf)
  #  rf2 = RangeFilter.new_more("int", "3")
  #  do_test_top_docs(is, query, [4,6], rf2)
  #end
end
