require File.dirname(__FILE__) + "/../../test_helper"

include Ferret::Index
include Ferret::Analysis

class SegmentTermDocEnumTest < Test::Unit::TestCase
  def setup()
    @dir = RAMDirectory.new
    @doc = IndexTestHelper.prepare_document()
    IndexTestHelper.write_document(@dir, @doc)
  end

  def test_something()
    assert true
  end
end
