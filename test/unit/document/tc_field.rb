require File.dirname(__FILE__) + "/../../test_helper"

include Ferret::Document
include Ferret::Utils

class FieldTest < Test::Unit::TestCase
  def test_store()
    assert_equal("COMPRESS", Field::Store::COMPRESS.to_s)
    assert_equal("YES", Field::Store::YES.to_s)
    assert_equal("NO", Field::Store::NO.to_s)
  end

  def test_index()
    assert_equal("TOKENIZED", Field::Index::TOKENIZED.to_s)
    assert_equal("UNTOKENIZED", Field::Index::UNTOKENIZED.to_s)
    assert_equal("NO", Field::Index::NO.to_s)
  end

  def test_term_vector()
    assert_equal("YES", Field::TermVector::YES.to_s)
    assert_equal("NO", Field::TermVector::NO.to_s)
    assert_equal("WITH_POSITIONS", Field::TermVector::WITH_POSITIONS.to_s)
    assert_equal("WITH_OFFSETS", Field::TermVector::WITH_OFFSETS.to_s)
    assert_equal("WITH_POSITIONS_OFFSETS", Field::TermVector::WITH_POSITIONS_OFFSETS.to_s)
  end

  def test_standard_field()
    f = Field.new("name", "value", Field::Store::COMPRESS, Field::Index::TOKENIZED)
    assert_equal("name", f.name)
    assert_equal("value", f.data)
    assert_equal(true, f.stored?)
    assert_equal(true, f.compressed?)
    assert_equal(true, f.indexed?)
    assert_equal(true, f.tokenized?)
    assert_equal(false, f.store_term_vector?)
    assert_equal(false, f.store_offsets?)
    assert_equal(false, f.store_positions?)
    assert_equal(false, f.binary?)
  end

  def test_set_store()
    f = Field.new("name", "value", Field::Store::COMPRESS, Field::Index::TOKENIZED)
    f.stored = Field::Store::NO
    assert_equal(false, f.stored?)
    assert_equal(false, f.compressed?)
  end

  def test_set_index()
    f = Field.new("name", "value", Field::Store::COMPRESS, Field::Index::TOKENIZED)
    f.index = Field::Index::NO
    assert_equal(false, f.indexed?)
    assert_equal(false, f.tokenized?)
  end

  def test_set_term_vector()
    f = Field.new("name", "value", Field::Store::COMPRESS, Field::Index::TOKENIZED)
    f.store_term_vector = Field::TermVector::WITH_POSITIONS_OFFSETS
    assert_equal(true, f.store_term_vector?)
    assert_equal(true, f.store_offsets?)
    assert_equal(true, f.store_positions?)
  end

  def test_new_binary_field()
    tmp = []
    256.times {|i| tmp[i] = i}
    bin = tmp.pack("c*")
    f = Field.new_binary_field("name", bin, Field::Store::YES)
    assert_equal("name", f.name)
    assert_equal(bin, f.data)
    assert_equal(true, f.stored?)
    assert_equal(false, f.compressed?)
    assert_equal(false, f.indexed?)
    assert_equal(false, f.tokenized?)
    assert_equal(false, f.store_term_vector?)
    assert_equal(false, f.store_offsets?)
    assert_equal(false, f.store_positions?)
    assert_equal(true, f.binary?)
  end
end
