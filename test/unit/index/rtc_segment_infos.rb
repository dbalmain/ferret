require File.dirname(__FILE__) + "/../../test_helper"


class SegmentInfosTest < Test::Unit::TestCase
  include Ferret::Index

  def setup()
    @dir = Ferret::Store::RAMDirectory.new
  end

  def tear_down()
    @dir.close()
  end

  def test_read_write
    assert_equal(0, SegmentInfos.read_current_version(@dir))
    sis = SegmentInfos.new()
    seg0 = SegmentInfo.new('seg0', 5, @dir)
    seg1 = SegmentInfo.new('seg1', 5, @dir)
    seg2 = SegmentInfo.new('seg2', 5, @dir)
    seg3 = SegmentInfo.new('seg3', 5, @dir)
    sis << seg0
    sis << seg1
    sis << seg2
    assert_equal(sis.size(), 3)
    assert_equal(sis[0], seg0)
    assert_equal(sis[2], seg2)
    sis.write(@dir)
    version = SegmentInfos.read_current_version(@dir)
    assert(@dir.exists?('segments'))
    sis2 = SegmentInfos.new()
    sis2.read(@dir)
    assert_equal(sis2.size(), 3)
    assert_equal(sis2[0], seg0)
    assert_equal(sis2[2], seg2)
    sis2 << seg3
    sis2.write(@dir)
    assert_equal(version + 1, SegmentInfos.read_current_version(@dir))
    sis3 = SegmentInfos.new()
    sis3.read(@dir)
    assert_equal(sis3.size(), 4)
    assert_equal(sis2[0], seg0)
    assert_equal(sis2[3], seg3)
  end
end

class SegmentInfoTest < Test::Unit::TestCase
  include Ferret::Index

  def setup()
    @dir = Ferret::Store::RAMDirectory.new
  end

  def tear_down()
    @dir.close()
  end

  # just test getters and setters. Nothing else.
  def test_segment_info
    si = SegmentInfo.new("seg1", 0, @dir)
    assert_equal(si.directory, @dir)
    assert_equal(si.doc_count, 0)
    assert_equal(si.name, "seg1")
    @dir.close()
    @dpath = File.dirname(__FILE__) + '/../../temp/fsdir'
    @dir = Ferret::Store::FSDirectory.new(@dpath, true)
    si.name = "seg2"
    si.doc_count += 2
    si.directory = @dir
    assert_equal(si.directory, @dir)
    assert_equal(si.doc_count, 2)
    assert_equal(si.name, "seg2")
  end
end
