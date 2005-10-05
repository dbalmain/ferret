require File.dirname(__FILE__) + "/../../test_helper"

include Ferret::Document

class DateToolsTest < Test::Unit::TestCase
  def test_serialization()
    # grab time to the nearest millisecond
    t = Time.at((Time.now().to_i*1000).floor()/1000)

    s = DateTools.serialize_time(t)

    t_after = DateTools.deserialize_time(s)
    assert_equal(t, t_after, "date changed after serialization")
  end
  
  def test_serialization_constants()
    # assert existance of these constants
    assert(DateTools::MAX_SERIALIZED_DATE_STRING)
    assert(DateTools::MIN_SERIALIZED_DATE_STRING)
  end

  def test_time_to_s()
    t = Time.mktime(2004, 9, 5, 22, 33, 44, 555000)

    assert_equal("2004", DateTools.time_to_s(t, DateTools::Resolution::YEAR))
    assert_equal("200409", DateTools.time_to_s(t, DateTools::Resolution::MONTH))
    assert_equal("20040905", DateTools.time_to_s(t, DateTools::Resolution::DAY))
    assert_equal("2004090522", DateTools.time_to_s(t, DateTools::Resolution::HOUR))
    assert_equal("200409052233", DateTools.time_to_s(t, DateTools::Resolution::MINUTE))
    assert_equal("20040905223344", DateTools.time_to_s(t, DateTools::Resolution::SECOND))
    assert_equal("20040905223344555", DateTools.time_to_s(t, DateTools::Resolution::MILLISECOND))
  end

  def test_s_to_time()
    assert_equal(Time.mktime(2004), DateTools.s_to_time("2004"))
    assert_equal(Time.mktime(2004, 9), DateTools.s_to_time("200409"))
    assert_equal(Time.mktime(2004, 9, 5), DateTools.s_to_time("20040905"))
    assert_equal(Time.mktime(2004, 9, 5, 22), DateTools.s_to_time("2004090522"))
    assert_equal(Time.mktime(2004, 9, 5, 22, 33), DateTools.s_to_time("200409052233"))
    assert_equal(Time.mktime(2004, 9, 5, 22, 33, 44), DateTools.s_to_time("20040905223344"))
    assert_equal(Time.mktime(2004, 9, 5, 22, 33, 44, 555000), DateTools.s_to_time("20040905223344555"))
  end

  def test_round()
    t = Time.mktime(2004, 9, 5, 22, 33, 44, 555000)
    assert_equal(Time.mktime(2004, 9, 5), DateTools.round(t, DateTools::Resolution::DAY))
  end

end
