require File.dirname(__FILE__) + "/../../test_helper"


class FieldTest < Test::Unit::TestCase
  include Ferret::Document
  include Ferret::Utils

  def test_store()
    assert_not_nil(Field::Store::COMPRESS)
    assert_not_nil(Field::Store::YES)
    assert_not_nil(Field::Store::NO)
  end

  def test_index()
    assert_not_nil(Field::Index::TOKENIZED)
    assert_not_nil(Field::Index::UNTOKENIZED)
    assert_not_nil(Field::Index::NO)
    assert_not_nil(Field::Index::NO_NORMS)
  end

  def test_term_vector()
    assert_not_nil(Field::TermVector::YES)
    assert_not_nil(Field::TermVector::NO)
    assert_not_nil(Field::TermVector::WITH_POSITIONS)
    assert_not_nil(Field::TermVector::WITH_OFFSETS)
    assert_not_nil(Field::TermVector::WITH_POSITIONS_OFFSETS)
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
    assert_equal(false, f.omit_norms?)
    assert_equal(false, f.binary?)
    assert_equal("stored/compressed,indexed,tokenized,<name:value>", f.to_s)
    f.data = "183"
    f.boost = 0.001
    assert_equal("183", f.data)
    assert(0.001 =~ f.boost)
  end

  def test_set_store()
    f = Field.new("name", "", Field::Store::COMPRESS, Field::Index::TOKENIZED)
    f.store = Field::Store::NO
    assert_equal(false, f.stored?)
    assert_equal(false, f.compressed?)
    assert_equal("indexed,tokenized,<name:>", f.to_s)
  end

  def test_set_index()
    f = Field.new("name", "value", Field::Store::COMPRESS, Field::Index::TOKENIZED)
    f.index = Field::Index::NO
    assert_equal(false, f.indexed?)
    assert_equal(false, f.tokenized?)
    assert_equal(false, f.omit_norms?)
    assert_equal("stored/compressed,<name:value>", f.to_s)
    f.index = Field::Index::NO_NORMS
    assert_equal(true, f.indexed?)
    assert_equal(false, f.tokenized?)
    assert_equal(true, f.omit_norms?)
    assert_equal("stored/compressed,indexed,omit_norms,<name:value>", f.to_s)
  end

  def test_set_term_vector()
    f = Field.new("name", "value", Field::Store::COMPRESS, Field::Index::TOKENIZED)
    f.term_vector = Field::TermVector::WITH_POSITIONS_OFFSETS
    assert_equal(true, f.store_term_vector?)
    assert_equal(true, f.store_offsets?)
    assert_equal(true, f.store_positions?)
    assert_equal("stored/compressed,indexed,tokenized,store_term_vector,store_offsets,store_positions,<name:value>", f.to_s)
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
    assert_equal(false, f.omit_norms?)
    assert_equal(true, f.binary?)
    assert_equal("stored/uncompressed,binary,<name:=bin_data=>", f.to_s)
  end
end
