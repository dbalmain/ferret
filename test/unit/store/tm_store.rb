module StoreTest
  # declare dir so inheritors can access it.
  @dir = nil

  # test the basic file manipulation methods;
  # - exists?
  # - touch
  # - delete
  # - file_count
  def test_basic_file_ops
    assert_equal(0, @dir.file_count(), "directory should be empty")
    assert(! @dir.exists?('filename'), "File should not exist")
    @dir.touch('tmpfile1')
    assert_equal(1, @dir.file_count(), "directory should have one file")
    @dir.touch('tmpfile2')
    assert_equal(2, @dir.file_count(), "directory should have two files")
    assert(@dir.exists?('tmpfile1'), "'tmpfile1' should exist")
    @dir.delete('tmpfile1')
    assert(! @dir.exists?('tmpfile1'), "'tmpfile1' should no longer exist")
    assert_equal(1, @dir.file_count(), "directory should have one file")
  end
  
  def test_rename
    @dir.touch("from")
    assert(@dir.exists?('from'), "File should exist")
    assert(! @dir.exists?('to'), "File should not exist")
    cnt_before = @dir.file_count()
    @dir.rename('from', 'to')
    cnt_after = @dir.file_count()
    assert_equal(cnt_before, cnt_after, "the number of files shouldn't have changed")
    assert(@dir.exists?('to'), "File should now exist")
    assert(! @dir.exists?('from'), "File should no longer exist")
  end

  def test_modified
    # difficult to test this one but as file mtime is only stored to the nearest second.
    # we can assume this test will happen in less than a few seconds. (I hope)
    time = Time.new.to_i
    @dir.touch('mtime.test')
    time_before = @dir.modified('mtime.test').to_i
    assert(time_before - time <= 3,
           "test that mtime is approximately equal to the system time when the file was touched")
  end

  def test_rw_bytes
    bytes = [0x34, 0x87, 0xF9, 0xEA, 0x00, 0xFF]
    rw_test(bytes, "byte", 6)
  end

  def test_rw_ints
    ints = [-2147483648, 2147483647, -1, 0]
    rw_test(ints, "int", 16)
  end

  def test_rw_longs
    longs = [-9223372036854775808, 9223372036854775807, -1, 0]
    rw_test(longs, "long", 32)
  end

  def test_rw_uints
    uints = [0xffffffff, 100000, 0]
    rw_test(uints, "uint", 12)
  end

  def test_rw_ulongs
    ulongs = [0xffffffffffffffff, 100000000000000, 0]
    rw_test(ulongs, "ulong", 24)
  end

  def test_rw_vints
    vints = [ 9223372036854775807,
              0x00,
              0xFFFFFFFFFFFFFFFF]
    rw_test(vints, "vint", 20)
  end

  def test_rw_vlongs
    vlongs = [ 9223372036854775807,
               0x00,
               0xFFFFFFFFFFFFFFFF]
    rw_test(vlongs, "vlong", 20)
  end

  def test_rw_strings
    text = 'This is a ruby ferret test string ~!@#$%^&*()`123456790-=\)_+|'
    ostream = @dir.create_output("rw_strings.test")
    ostream.write_string(text)
    ostream.write_string(text*100)
    ostream.close
    istream = @dir.open_input("rw_strings.test")
    assert_equal(text, istream.read_string, "Short string test failed")
    assert_equal(text*100, istream.read_string, "Short string test failed")
    istream.close
    assert_equal(6265, @dir.length('rw_strings.test'))
  end

  def test_rw_utf8_strings
    text = '³³ ÄÄÄÄÄÄ 道德經'
    ostream = @dir.create_output("rw_utf8_strings.test")
    ostream.write_string(text)
    ostream.write_string(text*100)
    ostream.close
    istream = @dir.open_input("rw_utf8_strings.test")
    assert_equal(text, x = istream.read_string, "Short string test failed")
    assert_equal(text*100, istream.read_string, "Short string test failed")
    istream.close
  end

  # this test fills up the output stream so that the buffer will have to be
  # written a few times. It then uses seek to make sure that it works
  # correctly
  def test_buffer_seek
    ostream = @dir.create_output("rw_seek.test")
    text = 'This is another long test string !@#$%#$%&%$*%^&*()(_'
    1000.times {|i| ostream.write_long(i); ostream.write_string(text) }
    ostream.seek(987)
    assert_equal(987, ostream.pos)
    ostream.write_vint(555)
    ostream.seek(56)
    assert_equal(56, ostream.pos)
    ostream.write_vint(1234567890)
    ostream.seek(4000)
    assert_equal(4000, ostream.pos)
    ostream.write_vint(9876543210)
    ostream.close()
    istream = @dir.open_input("rw_seek.test")
    istream.seek(56)
    assert_equal(56, istream.pos)
    assert_equal(1234567890, istream.read_vint())
    istream.seek(4000)
    assert_equal(4000, istream.pos)
    assert_equal(9876543210, istream.read_vint())
    istream.seek(987)
    assert_equal(987, istream.pos)
    assert_equal(555, istream.read_vint())
    istream.close()
  end

  def test_clone
    ostream = @dir.create_output("clone_test")
    10.times {|i| ostream.write_long(i) }
    ostream.close
    istream = @dir.open_input("clone_test")
    istream.seek(24)
    alt_istream = istream.clone
    assert_equal(istream.pos, alt_istream.pos)
    (3...10).each {|i| assert_equal(i, alt_istream.read_long) }
    assert_equal(80, alt_istream.pos)
    assert_equal(24, istream.pos)
    alt_istream.close
    (3...10).each {|i| assert_equal(i, istream.read_long) }
    istream.close
  end

  def test_read_bytes
    str = "0000000000"
    ostream = @dir.create_output("rw_read_bytes")
    ostream.write_bytes("how are you doing?", 18)
    ostream.close
    istream = @dir.open_input("rw_read_bytes")
    istream.read_bytes(str, 2, 4)
    assert_equal("00how 0000", str)
    istream.read_bytes(str, 1, 8)
    assert_equal("0are you 0", str)
    istream.close
  end

  private

  def rw_test(values, type, expected_length)
    ostream = @dir.create_output("rw_#{type}.test")
    values.each { |b| ostream.__send__("write_" + type, b) }
    ostream.close
    istream = @dir.open_input("rw_#{type}.test")
    values.each { |b| assert_equal(b, istream.__send__("read_" + type), "#{type} should be equal") }
    istream.close
    assert_equal(expected_length, @dir.length("rw_#{type}.test"))
  end

end
