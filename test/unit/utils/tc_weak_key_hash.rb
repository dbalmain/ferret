require File.dirname(__FILE__) + "/../../test_helper"


class WeakKeyHashTest < Test::Unit::TestCase
  include Ferret::Utils

  def test_objects_are_destroyed()
    w = WeakKeyHash.new()
    a = []
    10.times {|i| a[i] = "#{i}"; w[a[i]] = i }
    10.times {|i| assert_equal(i, w[a[i]]) }
    assert_equal(10, w.size)
    10.times {|i| a[i] = nil; }
    #puts w
    
    # this is a hack to get the GC to collect the last ref created above
    x = WeakKeyHash.new()
    10.times {|i| a[i] = "#{i}"; x[a[i]] = i }

    GC.start
    #puts w.size
    #puts w
    assert(0, w.size)
  end
end
