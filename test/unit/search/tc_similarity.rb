require File.dirname(__FILE__) + "/../../test_helper"

include Ferret::Search

class SimilarityTest < Test::Unit::TestCase
  def test_byte_float_conversion()
    256.times do |i|
      assert_equal(i, Similarity.float_to_byte(Similarity.byte_to_float(i)))
      assert_equal(Similarity.byte_to_float(i), Similarity::NORM_TABLE[i])
      assert_equal(i, Similarity.encode_norm(Similarity.decode_norm(i)))
    end
  end

  def test_default_similarity
    dsim = DefaultSimilarity.new()
    assert_equal(1.0/4, dsim.length_norm("field", 16))
    assert_equal(1.0/4, dsim.query_norm(16))
    assert_equal(3.0, dsim.tf(9))
    assert_equal(1.0/10, dsim.sloppy_freq(9))
    assert_equal(1.0, dsim.idf(9, 10))
    assert_equal(4.0, dsim.coord(12, 3))
    searcher = Object.new
    def searcher.doc_freq(term) 9 end
    def searcher.max_doc() 10 end
    term = Term.new("field", "text")
    assert_equal(1.0, dsim.idf_term(term, searcher))
    terms = [
      Term.new("field1", "text1"),
      Term.new("field1", "text2"),
      Term.new("field2", "text3"),
      Term.new("field2", "text4")
    ]
    assert_equal(4.0, dsim.idf_phrase(terms, searcher))
  end
end
