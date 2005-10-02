require File.dirname(__FILE__) + "/../../test_helper"

include Ferret::Index
include Ferret::Document
include Ferret::Store

class FieldsWriterTest < Test::Unit::TestCase

  def setup()
    @dir = RAMDirectory.new
  end

  def tear_down()
    @dir.close()
  end

  def test_writer
    doc = Document.new
    doc << Field.new("name", "daily news")
    doc << Field.new("content", "Nothing happened today.")
    
    infos = FieldInfos.new
    infos << doc
    
    writer = FieldsWriter.new(@dir, "fieldswritertest", infos)
    writer << doc
    writer.close

    fstream = @dir.open_input("fieldswritertest.fdt")
    istream = @dir.open_input("fieldswritertest.fdx")

    stored = fstream.read_vint
		field_num1 = fstream.read_vint
		byte1 = fstream.read_byte
		data1 = fstream.read_string
		assert( stored == 2 )
		assert( (byte1 |= FieldsWriter::FIELD_IS_TOKENIZED) != 0 )
		assert( data1 == "daily news" )
		
		field_num2 = fstream.read_vint
		byte2 = fstream.read_byte
		data2 = fstream.read_string
		assert( (byte2 |= FieldsWriter::FIELD_IS_TOKENIZED) != 0 )
		assert( data2 == "Nothing happened today." )

  end
end

class FieldsReaderTest < Test::Unit::TestCase

  def setup()
    @dir = RAMDirectory.new
  end

  def tear_down()
    @dir.close()
  end

  def test_doc
    doc = Document.new
    doc << Field.new("name", "daily news")
    doc << Field.new("content", "Nothing happened today.")
    
    infos = FieldInfos.new
    infos << doc

    fstream = @dir.create_output("fieldsreadertest.fdt")
    istream = @dir.create_output("fieldsreadertest.fdx")

    istream.write_long(0)
    istream.close
    fstream.write_vint(2)
    fstream.write_vint(0)
    fstream.write_byte(0)
    fstream.write_string("daily news")
    fstream.write_vint(1)
    fstream.write_byte(0)
    fstream.write_string("Nothing happened today.")
    fstream.close

    reader = FieldsReader.new(@dir, "fieldsreadertest", infos)
    docres = reader.doc(0)
    
    assert_equal(docres.field("name").data, "daily news")
    assert_equal(docres.field("content").data, "Nothing happened today.")
  end
end

class FieldsIOTest < Test::Unit::TestCase
  def setup()
    @dir = RAMDirectory.new
    doc = IndexTestHelper.prepare_document()
    infos = FieldInfos.new
    infos << doc

    writer = FieldsWriter.new(@dir, "field_types", infos)
    writer << doc
    writer.close

    reader = FieldsReader.new(@dir, "field_types", infos)
    @docres = reader.doc(0)
  end

  def tear_down()
    @dir.close()
  end

  def test_text_field_no_term_vector
    field = @docres.field("text_field1")
    check_field_values(field, "field one text", true, true, true, false, false)
  end

  def test_text_field_term_vector
    field = @docres.field("text_field2")
    check_field_values(field, "field field field two text", true, true, true, true, false)
  end

  def test_key_field
    field = @docres.field("key_field")
    check_field_values(field, "keyword", true, true, false, false, false)
  end

  def test_unindexed_field
    field = @docres.field("unindexed_field")
    check_field_values(field, "unindexed field text", true, false, false, false, false)
  end

  def test_unstored_field_no_term_vector
    field = @docres.field("unstored_field1")
    assert_equal(nil, field)
  end

  def test_compressed_field
    field = @docres.field("compressed_field")
    check_field_values(field, "compressed text", true, true, true, true, false)
  end

  def test_binary_field
    bin = IndexTestHelper::BINARY_DATA
    field = @docres.field("binary_field")
    check_field_values(field, bin, true, false, false, false, true)
  end

  def test_compressed_binary_field
    cbin = IndexTestHelper::COMPRESSED_BINARY_DATA
    field = @docres.field("compressed_binary_field")
    check_field_values(field, cbin, true, false, false, false, true)
  end


  private
  
  def check_field_values(field, value, stored, indexed, tokenized, term_vector, binary)
    assert_equal(value, field.data)
    assert_equal(stored, field.stored?)
    assert_equal(indexed, field.indexed?)
    assert_equal(tokenized, field.tokenized?)
    assert_equal(term_vector, field.store_term_vector?)
    assert_equal(binary, field.binary?)
  end
end
