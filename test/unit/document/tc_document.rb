require File.dirname(__FILE__) + "/../../test_helper"

include Ferret::Document

class DocumentTest < Test::Unit::TestCase
  def test_document()
    doc = Document.new()
    f11 = Field.new("field1", "value1", Field::Store::YES, Field::Index::NO)
    f12 = Field.new("field1", "value2", Field::Store::YES, Field::Index::NO)
    f13 = Field.new("field1", "value3", Field::Store::YES, Field::Index::NO)
    f21 = Field.new("field2", "value1", Field::Store::YES, Field::Index::NO)
    doc.add_field(f11)
    doc.add_field(f12)
    doc.add_field(f13)
    doc.add_field(f21)
    assert_equal(3, doc.fields("field1").size)
    assert_equal(1, doc.fields("field2").size)
    field = doc.remove_field("field1")
    assert_equal(2, doc.fields("field1").size)
    assert_equal(f11, field)
    values = doc.values("field1")
    assert_equal(["value2", "value3"], values)
    doc.remove_fields("field1")
    assert_equal(nil, doc.field("field1"))
  end

  def test_binary_string()
    doc = Document.new()
    fs1 = Field.new("field1", "value1", Field::Store::YES, Field::Index::NO)
    fs2 = Field.new("field1", "value2", Field::Store::YES, Field::Index::NO)
    fb1 = Field.new_binary_field("field1", "value1".intern, Field::Store::YES)
    fb2 = Field.new_binary_field("field1", "value2".intern, Field::Store::YES)

    doc.add_field(fs1)
    doc.add_field(fs2)
    doc.add_field(fb1)
    doc.add_field(fb2)

    assert_equal(4, doc.fields("field1").size)
    assert_equal(["value1", "value2"], doc.values("field1"))
    assert_equal(["value1".intern, "value2".intern], doc.binaries("field1"))
  end
end
