require File.dirname(__FILE__) + "/../../test_helper"


class CompoundFileWriterTest < Test::Unit::TestCase

  include Ferret::Index

  def setup()
    @dir = Ferret::Store::RAMDirectory.new
  end

  def tear_down()
    @dir.close()
  end

  def test_writer
    file1 = @dir.create_output("file1")
    file2 = @dir.create_output("file2")
    file1.write_int(20)
    file2.write_string('this is file2')
    file1.close()
    file2.close()
    cfile_writer = CompoundFileWriter.new(@dir, "cfile")
    cfile_writer.add_file("file1")
    cfile_writer.add_file("file2")
    cfile_writer.close()
    
    cfile = @dir.open_input("cfile")
    assert_equal(2, cfile.read_vint())
    assert_equal(29, cfile.read_long(), "Offset is incorrect")
    assert_equal("file1", cfile.read_string(), "Filename is incorrect")
    assert_equal(33, cfile.read_long(), "Offset is incorrect")
    assert_equal("file2", cfile.read_string(), "Filename is incorrect")
    assert_equal(20, cfile.read_int(), "Content is incorrect")
    assert_equal('this is file2', cfile.read_string(), "Content is incorrect")
  end
end

class CompoundFileReaderTest < Test::Unit::TestCase

  include Ferret::Index

  def setup()
    @dir = Ferret::Store::RAMDirectory.new
  end

  def tear_down()
    @dir.close()
  end

  def test_reader
    cfile = @dir.create_output("cfile")
    cfile.write_vint(2)
    cfile.write_long(29)
    cfile.write_string('file1')
    cfile.write_long(33)
    cfile.write_string('file2')
    cfile.write_int(20)
    cfile.write_string("this is file 2")
    cfile.close()
    
    cfile_reader = CompoundFileReader.new(@dir, "cfile")
    assert_equal(4, cfile_reader.length('file1'))
    assert_equal(15, cfile_reader.length('file2'))
    file1 = cfile_reader.open_input('file1')
    file2 = cfile_reader.open_input('file2')
    assert_equal(20, file1.read_int())
    assert_equal('this is file 2', file2.read_string())
    file1.close()
    file2.close()
  end
end

class CompoundFileIOTest < Test::Unit::TestCase

  include Ferret::Index

  def setup()
    @dir = Ferret::Store::RAMDirectory.new
  end

  def tear_down()
    @dir.close()
  end

  def test_buffer
    file1 = @dir.create_output("file1")
    file2 = @dir.create_output("file2")
    file3 = @dir.create_output("file3")
    20.times { file1.write_int(rand(10000)) }
    file2.write_string('this is file2' * 1000)
    file3.write_string('this is file2')
    file1.close()
    file2.close()
    file3.close()
    cfile_writer = CompoundFileWriter.new(@dir, "cfile")
    cfile_writer.add_file("file1")
    cfile_writer.add_file("file2")
    cfile_writer.add_file("file3")
    cfile_writer.close()
    
    cfile_reader = CompoundFileReader.new(@dir, "cfile")
    file2 = cfile_reader.open_input('file2')
    assert_equal('this is file2' * 1000, file2.read_string)
    file2.close
  end
end
