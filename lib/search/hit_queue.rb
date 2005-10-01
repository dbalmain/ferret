include Ferret::Utils
module Ferret::Search
  class HitQueue < PriorityQueue 
    def less_than(hit1, hit2) 
      if (hit1.score == hit2.score)
        return hit1.doc > hit2.doc
      else
        return hit1.score < hit2.score
      end
    end
  end
end
