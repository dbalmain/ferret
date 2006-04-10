require File.dirname(__FILE__) + "/../../test_helper"

class FuzzyQueryTest < Test::Unit::TestCase
  include Ferret::Document
  include Ferret::Search
  include Ferret::Store
  include Ferret::Analysis
  include Ferret::Index

  def add_doc(text, writer)
    doc = Document.new()
    doc << Field.new("field", text, Field::Store::NO, Field::Index::TOKENIZED)
    writer << doc
  end

  def setup()
    @dir = RAMDirectory.new()
  end

  def tear_down()
    @dir.close()
  end

  def do_test_top_docs(is, query, expected)
    top_docs = is.search(query)
    assert_equal(expected.length, top_docs.total_hits,
                "expected #{expected.length} hits but got #{top_docs.total_hits}")
    assert_equal(expected.length, top_docs.score_docs.size)
    top_docs.total_hits.times do |i|
      assert_equal(expected[i], top_docs.score_docs[i].doc)
    end
  end

  def do_prefix_test(is, text, prefix, expected)
    fq = FuzzyQuery.new(Term.new("field", text), FuzzyQuery.default_min_similarity, prefix)
    #puts is.explain(fq, 0)
    #puts is.explain(fq, 1)
    do_test_top_docs(is, fq, expected)
  end

  def test_fuzziness()
    iw = IndexWriter.new(@dir, :analyzer => WhiteSpaceAnalyzer.new(), :create => true)
    add_doc("aaaaa", iw)
    add_doc("aaaab", iw)
    add_doc("aaabb", iw)
    add_doc("aabbb", iw)
    add_doc("abbbb", iw)
    add_doc("bbbbb", iw)
    add_doc("ddddd", iw)
    add_doc("ddddddddddddddddddddd", iw) # test max_distances problem
    add_doc("aaaaaaaaaaaaaaaaaaaaaaa", iw) # test max_distances problem
    #iw.optimize()
    iw.close()


    is = IndexSearcher.new(@dir)

    fq = FuzzyQuery.new(Term.new("field", "aaaaa"), FuzzyQuery.default_min_similarity, 5)

    do_prefix_test(is, "aaaaaaaaaaaaaaaaaaaaaa", 1, [8])
    do_prefix_test(is, "aaaaa", 0, [0,1,2])
    do_prefix_test(is, "aaaaa", 1, [0,1,2])
    do_prefix_test(is, "aaaaa", 2, [0,1,2])
    do_prefix_test(is, "aaaaa", 3, [0,1,2])
    do_prefix_test(is, "aaaaa", 4, [0,1])
    do_prefix_test(is, "aaaaa", 5, [0])
    do_prefix_test(is, "aaaaa", 6, [0])

    do_prefix_test(is, "xxxxx", 0, [])

    do_prefix_test(is, "aaccc", 0, [])

    do_prefix_test(is, "aaaac", 0, [0,1,2])
    do_prefix_test(is, "aaaac", 1, [0,1,2])
    do_prefix_test(is, "aaaac", 2, [0,1,2])
    do_prefix_test(is, "aaaac", 3, [0,1,2])
    do_prefix_test(is, "aaaac", 4, [0,1])
    do_prefix_test(is, "aaaac", 5, [])

    do_prefix_test(is, "ddddX", 0, [6])
    do_prefix_test(is, "ddddX", 1, [6])
    do_prefix_test(is, "ddddX", 2, [6])
    do_prefix_test(is, "ddddX", 3, [6])
    do_prefix_test(is, "ddddX", 4, [6])
    do_prefix_test(is, "ddddX", 5, [])
    
    fq = FuzzyQuery.new(Term.new("anotherfield", "ddddX"), FuzzyQuery.default_min_similarity, 0)
    top_docs = is.search(fq)
    assert_equal(0, top_docs.total_hits)

    is.close()
  end

  def test_fuzziness_long()
    iw = IndexWriter.new(@dir, :analyzer => WhiteSpaceAnalyzer.new(), :create => true)
    add_doc("aaaaaaa", iw)
    add_doc("segment", iw)
    iw.optimize()
    iw.close()
    is = IndexSearcher.new(@dir)

    # not similar enough:
    do_prefix_test(is, "xxxxx", 0, [])

    # edit distance to "aaaaaaa" = 3, this matches because the string is longer than
    # in testDefaultFuzziness so a bigger difference is allowed:
    do_prefix_test(is, "aaaaccc", 0, [0])
    
    # now with prefix
    do_prefix_test(is, "aaaaccc", 1, [0])
    do_prefix_test(is, "aaaaccc", 4, [0])
    do_prefix_test(is, "aaaaccc", 5, [])

    # no match, more than half of the characters is wrong:
    do_prefix_test(is, "aaacccc", 0, [])
    
    # now with prefix
    do_prefix_test(is, "aaacccc", 1, [])

    # "student" and "stellent" are indeed similar to "segment" by default:
    do_prefix_test(is, "student", 0, [1])
    do_prefix_test(is, "stellent", 0, [1])

    # now with prefix
    do_prefix_test(is, "student", 2, [])
    do_prefix_test(is, "stellent", 2, [])

    # "student" doesn't match anymore thanks to increased minimum similarity:
    fq = FuzzyQuery.new(Term.new("field", "student"), 0.6, 0)
    top_docs = is.search(fq)
    assert_equal(0, top_docs.total_hits)

    assert_raise(ArgumentError) {fq = FuzzyQuery.new(Term.new("f", "s"), 1.1)}
    assert_raise(ArgumentError) {fq = FuzzyQuery.new(Term.new("f", "s"), -0.1)}

    is.close()
  end
  
end
