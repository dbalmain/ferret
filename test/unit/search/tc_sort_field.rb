require File.dirname(__FILE__) + "/../../test_helper"

class SortFieldTest < Test::Unit::TestCase
  def test_field_score()
    fs = SortField::FIELD_SCORE
    assert_equal(SortField::SortType::SCORE, fs.sort_type)
    assert_nil(fs.name)
    assert(!fs.reverse?, "FIELD_SCORE should not be reverse")
    assert_nil(fs.comparator)
  end

  def test_field_doc()
    fs = SortField::FIELD_DOC
    assert_equal(SortField::SortType::DOC, fs.sort_type)
    assert_nil(fs.name)
    assert(!fs.reverse?, "FIELD_DOC should not be reverse")
    assert_nil(fs.comparator)
  end

  def test_error_raised()
    assert_raise(ArgumentError) {
      fs = SortField.new(nil, {:sort_type => SortField::SortType::INT})
    }
  end
end
