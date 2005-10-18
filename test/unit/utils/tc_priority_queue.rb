require File.dirname(__FILE__) + "/../../test_helper"


class PriorityQueueTest < Test::Unit::TestCase
  include Ferret::Utils

  PQ_STRESS_SIZE = 1000

  def test_pq()
    pq = PriorityQueue.new(4)
    assert_equal(0, pq.size)
    pq.push("bword")
    assert_equal(1, pq.size)
    assert_equal("bword", pq.top)
    pq.push("cword")
    assert_equal(2, pq.size)
    assert_equal("bword", pq.top)
    pq.push("aword")
    assert_equal(3, pq.size)
    assert_equal("aword", pq.top)
    pq.push("dword")
    assert_equal(4, pq.size)
    assert_equal("aword", pq.top)
    assert_equal("aword", pq.pop())
    assert_equal(3, pq.size)
    assert_equal("bword", pq.pop())
    assert_equal(2, pq.size)
    assert_equal("cword", pq.pop())
    assert_equal(1, pq.size)
    assert_equal("dword", pq.pop())
    assert_equal(0, pq.size)
  end

  def test_pq_clear()
    pq = PriorityQueue.new(3)
    pq.push("word1")
    pq.push("word2")
    pq.push("word3")
    assert_equal(3, pq.size)
    pq.clear()
    assert_equal(0, pq.size)
  end


  #define PQ_STRESS_SIZE 1000
  def test_stress_pq()
    pq = PriorityQueue.new(PQ_STRESS_SIZE)
    PQ_STRESS_SIZE.times do
      pq.push("<#{(rand * PQ_STRESS_SIZE).to_i}>")
    end

    prev = pq.pop()
    (PQ_STRESS_SIZE - 1).times do
      curr = pq.pop()
      if (prev > curr)
        assert(prev <= curr, "previous #{prev} should be less than current #{curr}")
      end
      prev = curr
    end
    pq.clear()
  end
end
