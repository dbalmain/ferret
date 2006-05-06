require File.dirname(__FILE__) + "/../../test_helper"

class SortFieldTest < Test::Unit::TestCase
  include Ferret::Search

  def test_params()
    assert_equal("SCORE",   SortField::SortType::SCORE.to_s)
    assert_equal("DOC",     SortField::SortType::DOC.to_s)
    assert_equal("auto",    SortField::SortType::AUTO.to_s)
    assert_equal("string",  SortField::SortType::STRING.to_s)
    assert_equal("integer", SortField::SortType::INTEGER.to_s)
    assert_equal("float",   SortField::SortType::FLOAT.to_s)
  end
end
