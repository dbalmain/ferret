require File.dirname(__FILE__) + "/../../test_helper"

include Ferret::Utils

class WeakKeyHashTest < Test::Unit::TestCase
  def test_marshalling()
    w = WeakKeyHash.new()
    a = []
    10.times {|i| a[i] = "#{i}"; w[a[i]] = i }
    10.times {|i| assert_equal(i, w[a[i]]) }
    10.times {|i| a[i] = nil; }
    #puts w
    
    # this is a hack to get the GC to collect the last ref created above
    x = WeakKeyHash.new()
    10.times {|i| a[i] = "#{i}"; x[a[i]] = i }

    assert_equal(10, w.size)
    GC.start
    #puts w.size
    #puts w
    assert(0, w.size)
  end
end
