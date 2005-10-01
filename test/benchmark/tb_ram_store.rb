require File.dirname(__FILE__) + "/../test_helper"
include Ferret::Store
require 'benchmark'

class RAMStoreTest < Test::Unit::TestCase
  def setup
    @dir = RAMDirectory.new
  end

  def teardown
    @dir.close()
  end

  def test_rw_bytes
    bytes = [0x34, 0x87, 0xF9, 0xEA, 0x00, 0xFF]
    rw_test(bytes, "byte")
  end

  def test_rw_ints
    ints = [-2147483648, 2147483647, -1, 0]
    rw_test(ints, "int")
  end

  def test_rw_longs
    longs = [-9223372036854775808, 9223372036854775807, -1, 0]
    rw_test(longs, "long")
  end

  def test_rw_uints
    uints = [0xffffffff, 100000, 0]
    rw_test(uints, "uint")
  end

  def test_rw_ulongs
    ulongs = [0xffffffffffffffff, 100000000000000, 0]
    rw_test(ulongs, "ulong")
  end

  def test_rw_vints
    vints = [ 0xF8DC843342FE3484234987FE98AB987C897D214D123D123458EFBE2E238BACDEB9878790ABCDEF123DEF23988B89C,
              0x0000000000000000000000000000000000000000,
              0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF]
    rw_test(vints, "vint")
  end

  def test_rw_vlongs
    vlongs = [ 0xF8DC843342FE3484234987FE98AB987C897D214D123D123458EFBE2E238BACDEB9878790ABCDEF123DEF23988B89C,
              0x0000000000000000000000000000000000000000,
              0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF]
    rw_test(vlongs, "vlong")
  end

  def test_rw_strings
    strings = ['This is a ruby ferret test string ~!@#$%^&*()`123456790-=\)_+|', 'This is another string. I\'ll make this one a little longer than the last one. But I guess we need a few shorter ones too.', 'one', 'two', 'three', 'four', 'five', 'six', 'seven', 'eight', 'nine', 'ten']
    rw_test(strings, "string")
  end

  # this test fills up the output stream so that the buffer will have to be
  # written a few times. It then uses seek to make sure that it works
  # correctly

  def rw_test(values, type)
    puts "\nrw_#{type} test"
    Benchmark.bmbm do |x|
      x.report("write") do
        ostream = @dir.create_output("rw_#{type}.test")
        1000.times {values.each { |b| ostream.__send__("write_" + type, b) }}
        ostream.close
      end
      x.report("read") do
        istream = @dir.open_input("rw_#{type}.test")
        1000.times {values.each { |b| assert_equal(b, istream.__send__("read_" + type), "#{type} should be equal") }}
        istream.close
      end
    end
  end
end
