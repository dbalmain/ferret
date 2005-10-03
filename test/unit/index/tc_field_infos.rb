require File.dirname(__FILE__) + "/../../test_helper"
include Ferret::Index
include Ferret::Store

class FieldInfosTest < Test::Unit::TestCase
  def test_field_info()
    fi = FieldInfo.new("name", true, 1, true, true, true)
    assert_equal(fi.name, "name")
    assert_equal(fi.number, 1)
    assert(fi.indexed?)
    assert(fi.store_term_vector?)
    assert(fi.store_offsets?)
    assert(fi.store_positions?)

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
    assert(!fi.store_offsets?)
    assert(!fi.store_positions?)

    fi.set!(true, true, true, true)
    assert(fi.indexed?)
    assert(fi.store_term_vector?)
    assert(fi.store_offsets?)
    assert(fi.store_positions?)

    fi = FieldInfo.new("name", true, 1, true)
    assert(!fi.store_offsets?)
    assert(!fi.store_positions?)
  end

  def fi_test_attr(fi, name, number, indexed, store_tv, store_pos, store_off)
    assert_equal(name, fi.name)
    assert_equal(number, fi.number)
    assert_equal(indexed, fi.indexed?)
    assert_equal(store_tv, fi.store_term_vector?)
    assert_equal(store_pos, fi.store_positions?)
    assert_equal(store_off, fi.store_offsets?)
  end

  def test_fis_add()
    fis = FieldInfos.new()
    fi = fis.add("field1", false)
    fi_test_attr(fi, "field1", 0, false, false, false, false)
    assert_equal(1, fis.size)

    fi = fis.add("field1", true, true)
    fi_test_attr(fi, "field1", 0, true, true, false, false)
    assert_equal(1, fis.size)

    fi = fis.add("field2", false)
    fi_test_attr(fi, "field2", 1, false, false, false, false)
    assert_equal(2, fis.size)

    fi = fis.add("field1", true, true, true, true)
    assert_equal(fi, fis[fi.number])
    assert_equal(fi, fis["field1"])
    assert_equal(0, fis.field_number("field1"))
    assert_equal(1, fis.field_number("field2"))
    assert_equal(FieldInfos::NOT_A_FIELD, fis.field_number("field3"))
    assert_equal(nil, fis["field3"])
    fi_test_attr(fi, "field1", 0, true, true, true, true)
    assert_equal(2, fis.size)
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


  def test_fis_rw()
    fis = FieldInfos.new()
    dir = RAMDirectory.new()
    fis.add("field1", false, false, false, false)
    fis.add("field2", true, false, false, false)
    fis.add("field3", true, true, false, false)
    fis.add("field4", true, true, true, false)
    fis.add("field5", true, true, true, true)
    fis.write_to_dir(dir, "fis_rw.test")
    fis = nil

    fis = FieldInfos.new(dir, "fis_rw.test")
    fi_test_attr(fis[0], "field1", 0, false, false, false, false)
    fi_test_attr(fis[1], "field2", 1, true, false, false, false)
    fi_test_attr(fis[2], "field3", 2, true, true, false, false)
    fi_test_attr(fis[3], "field4", 3, true, true, true, false)
    fi_test_attr(fis[4], "field5", 4, true, true, true, true)

    assert_equal(5, fis.size)
  end

end
