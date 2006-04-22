require File.dirname(__FILE__) + "/../../test_helper"

# Tests the multisearcher by comparing it's results
# with those returned by an IndexSearcher.
# Taken from TestMultiSearcherRanking.java of Lucene
class MultiSearcher2Test < Test::Unit::TestCase
  include Ferret::Document
  include Ferret::Search
  include Ferret::Store
  include Ferret::Analysis
  include Ferret::Index

  FIELD_NAME = 'body'

  def test_one_Term_query
    check_query 'three'
  end

  def test_two_term_query
    check_query 'three foo'
    # as of 2006/03/11 these fail in Java Lucene as 
    # well, hits are returned in slightly different order. 
    #check_query '+pizza +blue*', :body
    #check_query '+pizza blue*', :body
    #check_query 'pizza blue*', :body
  end

  def test_prefix_query
    check_query 'multi*'
  end

  def test_fuzzy_query
    check_query 'multiThree~'
  end

  def test_range_query
    check_query '{multiA multiP}'
  end
  
  # fails (query parse error)
  #def test_multi_phrase_query
  #  check_query '"blueberry pi*"'
  #end
  
  def test_nomatch_query
    check_query '+three +nomatch'
  end
  
  # this yields differing scores, but doesn't work in 
  # Java Lucene either
  #def test_term_repeated_query
  #  check_query 'multi* multi* foo'
  #end
  
  
  def check_query(query_str, debug_field=nil)
    @parser ||= Ferret::QueryParser.new(FIELD_NAME, :analyzer => @analyzer)
    query = @parser.parse(query_str)
    puts "Query: #{query}" if debug_field
    IndexTestHelper.explain(query, @multi, debug_field) if debug_field
    IndexTestHelper.explain(query, @single, debug_field) if debug_field
    multi_hits = @multi.search(query)
    single_hits = @single.search(query)
    assert_equal single_hits.size, multi_hits.size, "hit count differs"
    multi_hits.score_docs.each_with_index { |multi_sd, i|
      single_sd = single_hits.score_docs[i]
      doc_multi = @multi.doc(multi_sd.doc)
      doc_single = @single.doc(single_sd.doc)
      assert_equal single_sd.score, multi_sd.score, "score differs in result #{i}"
      assert_equal doc_single[FIELD_NAME], doc_multi[FIELD_NAME], "field values differ in result #{i}"
    }
  end

  def setup()
    @analyzer = WhiteSpaceAnalyzer.new()
    # create MultiSearcher from two seperate searchers
    d1 = RAMDirectory.new()
    iw1 = IndexWriter.new(d1, :analyzer => @analyzer, :create => true)
    add_collection1(iw1)
    iw1.close()
    
    d2 = RAMDirectory.new()
    iw2 = IndexWriter.new(d2, :analyzer => @analyzer, :create => true)
    add_collection2(iw2)
    iw2.close()
    @multi = MultiSearcher.new([IndexSearcher.new(d1), IndexSearcher.new(d2)])

    # create IndexSearcher which contains all documents
    d = RAMDirectory.new()
    iw = IndexWriter.new(d, :analyzer => @analyzer, :create => true)
    add_collection1(iw)
    add_collection2(iw)
    iw.close()
    @single = IndexSearcher.new(d)
  end

  def tear_down()
    @multi.close
    @single.close
  end

  def add(value, iw)
    d = Document.new
    d << Field.new(FIELD_NAME, value, Field::Store::YES, Field::Index::TOKENIZED)
    iw << d
  end

  def add_collection1(iw)
    add("one blah three", iw)
    add("one foo three multiOne", iw)
    add("one foobar three multiThree", iw)
    add("blueberry pie", iw)
    add("blueberry strudel", iw)
    add("blueberry pizza", iw)
  end
  def add_collection2(iw)
    add("two blah three", iw)
    add("two foo xxx multiTwo", iw)
    add("two foobar xxx multiThreee", iw)
    add("blueberry chewing gum", iw)
    add("bluebird pizza", iw)
    add("bluebird foobar pizza", iw)
    add("piccadilly circus", iw)
  end

end
