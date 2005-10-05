module Ferret::Search
  class ExactPhraseScorer < PhraseScorer 

    def initialize(weight, tps, positions, similarity, norms) 
      super(weight, tps, positions, similarity, norms)
    end

    def phrase_freq()
      # sort list with pq
      each do |pp|
        pp.first_position()
        @pq.push(pp)         # build pq from list
      end
      pq_to_list()           # rebuild list from pq

      freq = 0
      begin # find position w/ all terms
        while (@first.position < @last.position) # scan forward in first
          begin
            if not @first.next_position()
              return freq
            end
          end while (@first.position < @last.position)
          first_to_last()
        end
        freq += 1            # all equal: a match
      end while @last.next_position()
    
      return freq
    end
  end
end
