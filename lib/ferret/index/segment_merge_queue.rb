module Ferret::Index
  class SegmentMergeQueue < Ferret::Utils::PriorityQueue 
    def less_than(sti_a, sti_b) 
      if sti_a.term_buffer == sti_b.term_buffer
        return sti_a.base < sti_b.base
      else
        return sti_a.term_buffer < sti_b.term_buffer
      end
    end

    def close()
      @heap.each {|sti| sti.close if sti}
      clear
    end
  end
end
