require File.dirname(__FILE__) + "/../../test_helper"

include Ferret::Index

class SegmentTermEnumTest < Test::Unit::TestCase
  TEST_SEGMENT = "_test"

  def setup()
    @dir = RAMDirectory.new
  end

  def test_initialize()
    fis = FieldInfos.new
    fis.add("author", true, true)
    fis.add("title", true, true)
    tiw = TermInfosWriter.new(@dir, TEST_SEGMENT, fis, 128)
    terms = [ Term.new("author", "Martel"),
              Term.new("title", "Life of Pi"),
              Term.new("author", "Martin"),
              Term.new("title", "Life on the edge") ].sort
    term_infos = []
    4.times {|i| term_infos << TermInfo.new(i,i,i,0)}
    4.times {|i| tiw.add(terms[i], term_infos[i]) }
    tiw.close()

    tis_file = @dir.open_input(TEST_SEGMENT + ".tis")
    
    ste = SegmentTermEnum.new(tis_file, fis, false)
    assert_equal(128, ste.index_interval)
    assert_equal(16, ste.skip_interval)
    assert_equal(4, ste.size)
    assert(ste.next?)
    assert_equal(terms[0], ste.term)
    assert_equal(term_infos[0], ste.term_info)
    ti = TermInfo.new
    ste.term_info = ti
    assert_equal(term_infos[0], ti)
    assert(ste.next?)
    assert_equal(terms[0], ste.prev)
    assert_equal(terms[1], ste.term)
    assert_equal(term_infos[1], ste.term_info)
    assert(ste.next?)
    assert_equal(terms[2], ste.term)
    assert_equal(term_infos[2], ste.term_info)
    assert(ste.next?)
    assert_equal(terms[3], ste.term)
    assert_equal(term_infos[3], ste.term_info)
    ste.close()

    tii_file = @dir.open_input(TEST_SEGMENT + ".tii")

    ste = SegmentTermEnum.new(tii_file, fis, false)
    assert_equal(128, ste.index_interval)
    assert_equal(16, ste.skip_interval)
    assert_equal(1, ste.size)
    assert(ste.next?)
    assert(Term.new("", ""), ste.term)
  end
end
