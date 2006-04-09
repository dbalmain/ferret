require File.dirname(__FILE__) + "/../../test_helper"


class FieldTest < Test::Unit::TestCase
  include Ferret::Document
  include Ferret::Utils

  def test_store()
    assert_equal("COMPRESS", Field::Store::COMPRESS.to_s)
    assert_equal("YES", Field::Store::YES.to_s)
    assert_equal("NO", Field::Store::NO.to_s)
  end

  def test_index()
    assert_equal("TOKENIZED", Field::Index::TOKENIZED.to_s)
    assert_equal("UNTOKENIZED", Field::Index::UNTOKENIZED.to_s)
    assert_equal("NO", Field::Index::NO.to_s)
    assert_equal("NO_NORMS", Field::Index::NO_NORMS.to_s)
  end

  def test_term_vector()
    assert_equal("YES", Field::TermVector::YES.to_s)
    assert_equal("NO", Field::TermVector::NO.to_s)
    assert_equal("WITH_POSITIONS", Field::TermVector::WITH_POSITIONS.to_s)
    assert_equal("WITH_OFFSETS", Field::TermVector::WITH_OFFSETS.to_s)
    assert_equal("WITH_POSITIONS_OFFSETS", Field::TermVector::WITH_POSITIONS_OFFSETS.to_s)
  end
end
