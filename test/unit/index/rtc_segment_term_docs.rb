require File.dirname(__FILE__) + "/../../test_helper"

class SegmentTermDocEnumTest < Test::Unit::TestCase

  include Ferret::Index
  include Ferret::Analysis

  def setup()
    @dir = Ferret::Store::RAMDirectory.new
    @doc = IndexTestHelper.prepare_document()
    IndexTestHelper.write_document(@dir, @doc)
  end

  def test_something()
    assert true
  end
end
