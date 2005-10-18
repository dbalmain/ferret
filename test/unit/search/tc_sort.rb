require File.dirname(__FILE__) + "/../../test_helper"

class SortTest < Test::Unit::TestCase
  include Ferret::Search

  def test_basic()
    s = Sort::RELEVANCE
    assert_equal(2, s.fields.size)
    assert_equal(SortField::FIELD_SCORE, s.fields[0])
    assert_equal(SortField::FIELD_DOC, s.fields[1])

    s = Sort::INDEX_ORDER
    assert_equal(1, s.fields.size)
    assert_equal(SortField::FIELD_DOC, s.fields[0])
  end

  def test_string_init()
    s = Sort.new("field")
    assert_equal(2, s.fields.size)
    assert_equal(SortField::SortType::AUTO, s.fields[0].sort_type)
    assert_equal("field", s.fields[0].name)
    assert_equal(SortField::FIELD_DOC, s.fields[1])

    s = Sort.new(["field1", "field2", "field3"])
    assert_equal(3, s.fields.size)
    assert_equal(SortField::SortType::AUTO, s.fields[0].sort_type)
    assert_equal("field1", s.fields[0].name)
    assert_equal(SortField::SortType::AUTO, s.fields[1].sort_type)
    assert_equal("field2", s.fields[1].name)
    assert_equal(SortField::SortType::AUTO, s.fields[2].sort_type)
    assert_equal("field3", s.fields[2].name)
  end

  def test_multi_fields()
    sf1 = SortField.new("field", {:sort_type => SortField::SortType::INT,
                                  :reverse => true})
    sf2 = SortField::FIELD_SCORE
    sf3 = SortField::FIELD_DOC
    s = Sort.new([sf1, sf2, sf3])

    assert_equal(3, s.fields.size)
    assert_equal(SortField::SortType::INT, s.fields[0].sort_type)
    assert_equal("field", s.fields[0].name)
    assert(s.fields[0].reverse?)
    assert_equal(SortField::FIELD_SCORE, s.fields[1])
    assert_equal(SortField::FIELD_DOC, s.fields[2])
  end
end
