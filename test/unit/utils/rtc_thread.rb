require File.dirname(__FILE__) + "/../../test_helper"
require 'thread'


class ThreadTest < Test::Unit::TestCase
  include Ferret::Utils

  NUM_THREADS = 100

  def test_basic_get_and_set()
    Thread.current.clear_local
    b = "hello"
    Thread.current.set_local(b, "dave")
    assert_equal("dave", Thread.current.get_local(b))
  end

  def test_objects_die
    Thread.current.clear_local
    a = []
    10.times {|i| a[i] = "#{i}"; Thread.current.set_local(a[i], i) }
    10.times {|i| assert_equal(i, Thread.current.get_local(a[i])) }
    assert_equal(10, Thread.current.local_size)
    GC.start
    assert_equal(10, Thread.current.local_size)
    10.times {|i| a[i] = nil; }
    #puts w
    
    # this is a hack to get the GC to collect the last ref created above
    x = WeakKeyHash.new()
    10.times {|i| a[i] = "#{i}"; x[a[i]] = i }

    assert_equal(10, Thread.current.local_size)
    GC.start
    assert(0, Thread.current.local_size)
  end

  class ThreadTester
    def initialize(val)
      Thread.current.set_local(self, val)
    end
    def inc
      val = Thread.current.get_local(self) + 1
      Thread.current.set_local(self, val)
      return val
    end
  end

  def single_thread
    tt = ThreadTester.new(start = rand(10000000))
    ((start+1)..start+11).each {|i| assert_equal(i, tt.inc) }
  end

  def test_threads_dont_share
    threads = []
    NUM_THREADS.times do
      threads << Thread.new { single_thread }
    end

    threads.each {|t| t.join}
  end
end
