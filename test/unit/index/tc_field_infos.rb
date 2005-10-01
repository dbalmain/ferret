require File.dirname(__FILE__) + "/../../test_helper"
require File.dirname(__FILE__) + "/th_doc"
include Ferret::Index
include Ferret::Store

class FieldInfosTest < Test::Unit::TestCase
  def test_field_info()
    fi = FieldInfo.new("name", true, 1, true, true, true)
    assert_equal(fi.name, "name")
    assert_equal(fi.number, 1)
    assert(fi.indexed?)
    assert(fi.store_term_vector?)
    assert(fi.store_offset?)
    assert(fi.store_position?)

    fi.name = "hello"
    fi.indexed = false
    fi.number = 2
    fi.store_term_vector = false
    fi.store_offset = false
    fi.store_position = false

    assert_equal(fi.name, "hello")
    assert_equal(fi.number, 2)
    assert(!fi.indexed?)
    assert(!fi.store_term_vector?)
    assert(!fi.store_offset?)
    assert(!fi.store_position?)

    fi.set!(true, true, true, true)
    assert(fi.indexed?)
    assert(fi.store_term_vector?)
    assert(fi.store_offset?)
    assert(fi.store_position?)

    fi = FieldInfo.new("name", true, 1, true)
    assert(!fi.store_offset?)
    assert(!fi.store_position?)
  end

  def test_add_doc_fields
    doc = IndexTestHelper.prepare_document
    fis = FieldInfos.new()
    fis << doc
    dir = RAMDirectory.new
    fis.write_to_dir(dir, "_test")
    fis2 = FieldInfos.new(dir, "_test")
    assert_equal("text_field1", fis2["text_field1"].name)
    fn = fis2.field_number("text_field2")
    assert_equal("text_field2", fis2[fn].name)
    assert_equal(9, fis2.size)
    assert(fis.has_vectors?)
  end

  def test_fis_has_vectors
    fis = FieldInfos.new()
    assert(! fis.has_vectors?)
    fis.add("random_field")
    assert(! fis.has_vectors?)
    fis.add("store_term_vector_field", true, true, false, false)
    assert(fis.has_vectors?)
  end
end
