module Ferret::Index
  class SegmentMergeQueue < Ferret::Utils::PriorityQueue 
    def less_than(sti_a, sti_b) 
      if sti_a.term == sti_b.term
        return sti_a.base < sti_b.base
      else
        return sti_a.term < sti_b.term
      end
    end

    def close()
      @heap.each {|sti| sti.close if sti}
      clear
    end
  end
end
