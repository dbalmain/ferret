require File.dirname(__FILE__) + "/../../test_helper"

include Ferret::Utils

class ParameterTest < Test::Unit::TestCase
  class Param1 < Parameter
    VAL1 = Param1.new("VAL")
  end

  class Param2 < Parameter
    VAL1 = Param2.new("VAL")
  end

  def test_parameter_cmp()
    assert_raise(ArgumentError) do
      class <<Param1
         v = Param1.new("VAL")
      end
    end
    assert_raise(ArgumentError) do
      class <<Param1
         v = Param2.new("VAL")
      end
    end
      
    p1 = Param1::VAL1
    p2 = Param1::VAL1
    p3 = Param2::VAL1
    assert_equal(p1, p2)
    assert_not_equal(p1, p3)
  end

  def test_marshalling()
    p1 = Param1::VAL1
    data = Marshal.dump(p1)
    p2 = Marshal.load(data)
    assert_equal(p1, p2)
  end
end
