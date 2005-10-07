require File.dirname(__FILE__) + "/../../test_helper"

class SortFieldTest < Test::Unit::TestCase
  def test_field_score()
    fs = SortField::FIELD_SCORE
    assert_equal(SortField::SortBy::SCORE, fs.type)
    assert_nil(fs.name)
    assert(!fs.reverse?, "FIELD_SCORE should not be reverse")
    assert_nil(fs.comparator)
    assert_nil(fs.locale)
  end

  def test_field_doc()
    fs = SortField::FIELD_DOC
    assert_equal(SortField::SortBy::DOC, fs.type)
    assert_nil(fs.name)
    assert(!fs.reverse?, "FIELD_DOC should not be reverse")
    assert_nil(fs.comparator)
    assert_nil(fs.locale)
  end
end
