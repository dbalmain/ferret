require File.dirname(__FILE__) + "/../../test_helper"

class SearchAndSortTest < Test::Unit::TestCase
  include Ferret::Document
  include Ferret::Search
  include Ferret::Store
  include Ferret::Analysis
  include Ferret::Index

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
    iw = IndexWriter.new(@dir, :analyzer => WhiteSpaceAnalyzer.new(), :create => true, :min_merge_docs => 3)
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

  def tear_down()
    @dir.close()
  end

  def do_test_top_docs(is, query, expected, sort = nil)
    top_docs = is.search(query, {:sort => sort})
    top_docs.total_hits.times do |i|
      assert_equal(expected[i], top_docs.score_docs[i].doc)
    end

    # test sorting works for smaller ranged query
    first_doc = 3
    num_docs = 3
    top_docs = is.search(query, {:sort => sort,
                                 :first_doc => first_doc,
                                 :num_docs => num_docs})
    num_docs.times do |i|
      assert_equal(expected[first_doc + i], top_docs.score_docs[i].doc)
    end
  end

  def test_sort_field_to_s()
    assert_equal("<SCORE>", SortField::FIELD_SCORE.to_s);
    sf = SortField.new("MyScore",
                       {:sort_type => SortField::SortType::SCORE,
                        :reverse => true})
    assert_equal("MyScore:<SCORE>!", sf.to_s)
    assert_equal("<DOC>", SortField::FIELD_DOC.to_s);
    sf = SortField.new("MyDoc",
                       {:sort_type => SortField::SortType::DOC,
                        :reverse => true})
    assert_equal("MyDoc:<DOC>!", sf.to_s)
    sf = SortField.new("date",
                       {:sort_type => SortField::SortType::INTEGER})
    assert_equal("date:<integer>", sf.to_s)
    sf = SortField.new("date",
                       {:sort_type => SortField::SortType::INTEGER,
                        :reverse => true})
    assert_equal("date:<integer>!", sf.to_s)
    sf = SortField.new("price",
                       {:sort_type => SortField::SortType::FLOAT})
    assert_equal("price:<float>", sf.to_s)
    sf = SortField.new("price",
                       {:sort_type => SortField::SortType::FLOAT,
                        :reverse => true})
    assert_equal("price:<float>!", sf.to_s)
    sf = SortField.new("content",
                       {:sort_type => SortField::SortType::STRING})
    assert_equal("content:<string>", sf.to_s)
    sf = SortField.new("content",
                       {:sort_type => SortField::SortType::STRING,
                        :reverse => true})
    assert_equal("content:<string>!", sf.to_s)
    sf = SortField.new("auto_field",
                       {:sort_type => SortField::SortType::AUTO})
    assert_equal("auto_field:<auto>", sf.to_s)
    sf = SortField.new("auto_field",
                       {:sort_type => SortField::SortType::AUTO,
                        :reverse => true})
    assert_equal("auto_field:<auto>!", sf.to_s)
  end
  
  def test_sort_to_s()
    sort = Sort.new
    assert_equal("Sort[<SCORE>, <DOC>]", sort.to_s)
    sf = SortField.new("auto_field",
                       {:sort_type => SortField::SortType::AUTO,
                        :reverse => true})
    sort = Sort.new([sf, SortField::FIELD_SCORE, SortField::FIELD_DOC])
    assert_equal("Sort[auto_field:<auto>!, <SCORE>, <DOC>]", sort.to_s)
    sort = Sort.new(["one", "two", SortField::FIELD_DOC])
    assert_equal("Sort[one:<auto>, two:<auto>, <DOC>]", sort.to_s)
    sort = Sort.new(["one", "two"])
    assert_equal("Sort[one:<auto>, two:<auto>, <DOC>]", sort.to_s)
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
    sf_int = SortField.new("int", {:sort_type => SortField::SortType::INTEGER, :reverse => true})
    do_test_top_docs(is, q, [0,1,6,5,9,4,8,2,7,3], [sf_int])
    do_test_top_docs(is, q, [0,1,6,5,9,8,4,7,2,3], [sf_int, SortField::FIELD_SCORE])
    sf_int = SortField.new("int", {:sort_type => SortField::SortType::INTEGER})
    do_test_top_docs(is, q, [3,2,7,4,8,5,9,1,6,0], [sf_int])

    ## float
    sf_float = SortField.new("float", {:sort_type => SortField::SortType::FLOAT, :reverse => true})
    do_test_top_docs(is, q, [8,7,5,3,1,0,2,4,6,9], Sort.new([sf_float, SortField::FIELD_SCORE]))
    sf_float = SortField.new("float", {:sort_type => SortField::SortType::FLOAT})
    do_test_top_docs(is, q, [9,6,4,2,0,1,3,5,7,8], Sort.new([sf_float, SortField::FIELD_SCORE]))

    ## str
    sf_str = SortField.new("string", {:sort_type => SortField::SortType::STRING})
    do_test_top_docs(is, q, [0,9,1,8,2,7,3,6,4,5], [sf_str, SortField::FIELD_SCORE])

    ## auto
    do_test_top_docs(is, q, [0,9,1,8,2,7,3,6,4,5], Sort.new("string"))
    do_test_top_docs(is, q, [3,2,7,4,8,5,9,1,6,0], Sort.new(["int"]))
    do_test_top_docs(is, q, [9,6,4,2,0,1,3,5,7,8], Sort.new("float"))
    do_test_top_docs(is, q, [9,6,4,2,0,1,3,5,7,8], "float")
    do_test_top_docs(is, q, [8,7,5,3,1,0,2,4,6,9], Sort.new("float", true))
    do_test_top_docs(is, q, [0,6,1,5,9,4,8,7,2,3], Sort.new(["int", "string"], true))
    do_test_top_docs(is, q, [3,2,7,8,4,9,5,1,6,0], Sort.new(["int", "string"]))
    do_test_top_docs(is, q, [3,2,7,8,4,9,5,1,6,0], [:int, "string"])
  end

  #LENGTH = SortField::SortType.new("length", lambda{|str| str.length})
  #LENGTH_MODULO = SortField::SortType.new("length_mod", lambda{|str| str.length},
  #                                        lambda{|i, j| (i%4) <=> (j%4)})
  #def test_special_sorts
  #  is = IndexSearcher.new(@dir)
  #  q = TermQuery.new(Term.new("search", "findall"))
  #  sf = SortField.new("float", {:sort_type => LENGTH, :reverse => true})
  #  do_test_top_docs(is, q, [9,6,4,8,2,7,0,5,1,3], [sf])
  #  sf = SortField.new("float", {:sort_type => LENGTH_MODULO, :reverse => true})
  #  do_test_top_docs(is, q, [1,3,6,4,8,2,7,0,5,9], [sf])
  #  sf = SortField.new("float", {:sort_type => LENGTH,
  #                               :reverse => true,
  #                               :comparator => lambda{|i,j| (j%4) <=> (i%4)}})
  #  do_test_top_docs(is, q, [0,5,9,2,7,4,8,1,3,6], [sf])
  #end
end
