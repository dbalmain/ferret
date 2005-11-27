require File.dirname(__FILE__) + "/../../test_helper"


class TermBufferTest < Test::Unit::TestCase
  include Ferret::Index
  def test_term_set()
    t = Term.new("title", "Ferret Tutorial")
    tb = TermBuffer.new
    tb.term = t
    assert_equal(t.field, tb.field)
    assert_equal("Ferret Tutorial", tb.text)
    assert_equal("Ferret Tutorial".length, tb.text_length)
    assert_equal(t, tb.term)
  end

  def test_set()
    tb = TermBuffer.new
    tb.term = Term.new("title", "Ferret Tutorial")
    tb2 = TermBuffer.new
    tb2.set!(tb)
    assert_equal(tb.field, tb2.field)
    assert_equal("Ferret Tutorial", tb2.text)
    assert_equal("Ferret Tutorial".length, tb2.text_length)
    assert_equal(tb.term, tb2.term)
  end

  def test_compare()
    tb1 = TermBuffer.new
    tb2 = TermBuffer.new
    tb1.term = Term.new("alpha", "text")
    tb2.term = Term.new("bravo", "text")
    assert(tb1 < tb2)
    tb2.term = Term.new("alpha", "text")
    assert(tb1 == tb2)
    tb2.term = Term.new("alpha", "tex")
    assert(tb1 > tb2)
  end

  def test_read()
    dir = Ferret::Store::RAMDirectory.new
    fi = FieldInfos.new
    tb = TermBuffer.new
    tb.term = Term.new("Author", "Dave")
    fi.add("Writer", true)
    output = dir.create_output("term_buffer_read_test")
    output.write_vint(4)
    output.write_vint(8)
    output.write_chars(" Balmain", 0, 8)
    output.write_vint(fi.field_number("Writer"))
    output.close
    input = dir.open_input("term_buffer_read_test")
    tb.read(input, fi)
    assert_equal("Dave Balmain", tb.text)
    assert_equal("Dave Balmain", tb.term.text)
    assert_equal("Writer", tb.field)
  end
end
