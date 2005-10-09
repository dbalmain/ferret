require File.dirname(__FILE__) + "/../../test_helper"

include Ferret::Search
include Ferret::Store
include Ferret::Analysis

class SearchAndSortTest < Test::Unit::TestCase
  def add_doc(hash, writer)
    doc = Document.new()
    hash.each_pair do |field, text|
      doc << Field.new(field, text, Field::Store::NO, Field::Index::UNTOKENIZED)
    end
    doc.boost = hash["float"].to_f
    writer << doc
  end

  def setup()
    @dir = RAMDirectory.new()
    iw = IndexWriter.new(@dir, WhiteSpaceAnalyzer.new(), true, false)
    docs = [                                                             # len mod
      {"search"=>"findall","string"=>"a","int"=>"6","float"=>"0.01"},    #  4   0
      {"search"=>"findall","string"=>"c","int"=>"5","float"=>"0.1"},     #  3   3
      {"search"=>"findall","string"=>"e","int"=>"2","float"=>"0.001"},   #  5   1
      {"search"=>"findall","string"=>"g","int"=>"1","float"=>"1.0"},     #  3   3
      {"search"=>"findall","string"=>"i","int"=>"3","float"=>"0.0001"},  #  6   2
      {"search"=>"findall","string"=>"j","int"=>"4","float"=>"10.0"},    #  4   0
      {"search"=>"findall","string"=>"h","int"=>"5","float"=>"0.00001"}, #  7   3
      {"search"=>"findall","string"=>"f","int"=>"2","float"=>"100.0"},   #  5   1
      {"search"=>"findall","string"=>"d","int"=>"3","float"=>"1000.0"},  #  6   2
      {"search"=>"findall","string"=>"b","int"=>"4","float"=>"0.000001"} #  8   0
    ]
    docs.each {|doc| add_doc(doc, iw)}
    iw.close
  end

  def test_special_sorts
    is = IndexSearcher.new(@dir)
    q = TermQuery.new(Term.new("search", "findall"))
    sf = SortField.new("float", {:sort_type => LENGTH})
    do_test_top_docs(is, q, [9,6,4,8,2,7,0,5,1,3], [sf])
    sf = SortField.new("float", {:sort_type => LENGTH_MODULO})
    do_test_top_docs(is, q, [1,3,6,4,8,2,7,0,5,9], [sf])
    sf = SortField.new("float", {:sort_type => LENGTH,
                                 :comparator => lambda{|i,j| (j%4) <=> (i%4)}})
    do_test_top_docs(is, q, [0,5,9,2,7,4,8,1,3,6], [sf])
  end

  def tear_down()
    @dir.close()
  end

  def do_test_top_docs(is, query, expected, sort = nil)
    top_docs = is.search(query, {:sort => sort})
    top_docs.total_hits.times do |i|
      assert_equal(expected[i], top_docs.score_docs[i].doc)
    end
  end

  def test_sorts()
    is = IndexSearcher.new(@dir)
    q = TermQuery.new(Term.new("search", "findall"))
    do_test_top_docs(is, q, [8,7,5,3,1,0,2,4,6,9])
    do_test_top_docs(is, q, [8,7,5,3,1,0,2,4,6,9], Sort::RELEVANCE)
    do_test_top_docs(is, q, [8,7,5,3,1,0,2,4,6,9], [SortField::FIELD_SCORE])
    do_test_top_docs(is, q, [0,1,2,3,4,5,6,7,8,9], Sort::INDEX_ORDER)
    do_test_top_docs(is, q, [0,1,2,3,4,5,6,7,8,9], [SortField::FIELD_DOC])

    ## int
    sf_int = SortField.new("int", {:sort_type => SortField::SortType::INT})
    do_test_top_docs(is, q, [0,1,6,5,9,4,8,2,7,3], [sf_int])
    do_test_top_docs(is, q, [0,1,6,5,9,8,4,7,2,3], [sf_int, SortField::FIELD_SCORE])
    sf_int = SortField.new("int", {:sort_type => SortField::SortType::INT, :reverse => true})
    do_test_top_docs(is, q, [3,2,7,4,8,5,9,1,6,0], [sf_int])

    ## float
    sf_float = SortField.new("float", {:sort_type => SortField::SortType::FLOAT})
    do_test_top_docs(is, q, [8,7,5,3,1,0,2,4,6,9], Sort.new([sf_float, SortField::FIELD_SCORE]))
    sf_float = SortField.new("float", {:sort_type => SortField::SortType::FLOAT, :reverse => true})
    do_test_top_docs(is, q, [9,6,4,2,0,1,3,5,7,8], Sort.new([sf_float, SortField::FIELD_SCORE]))

    ## str
    sf_str = SortField.new("string", {:sort_type => SortField::SortType::STRING})
    do_test_top_docs(is, q, [0,9,1,8,2,7,3,6,4,5], [sf_str, SortField::FIELD_SCORE])

    ## auto
    do_test_top_docs(is, q, [0,9,1,8,2,7,3,6,4,5], Sort.new("string"))
    do_test_top_docs(is, q, [0,1,6,5,9,4,8,2,7,3], Sort.new(["int"]))
    do_test_top_docs(is, q, [8,7,5,3,1,0,2,4,6,9], Sort.new("float"))
    do_test_top_docs(is, q, [9,6,4,2,0,1,3,5,7,8], Sort.new("float", true))
    do_test_top_docs(is, q, [0,1,6,9,5,8,4,2,7,3], Sort.new(["int", "string"]))
    do_test_top_docs(is, q, [3,7,2,4,8,5,9,6,1,0], Sort.new(["int", "string"], true))

  end

  LENGTH = SortField::SortType.new("length", lambda{|str| str.length})
  LENGTH_MODULO = SortField::SortType.new("length_mod", lambda{|str| str.length},
                                          lambda{|i, j| (i%4) <=> (j%4)})
end
