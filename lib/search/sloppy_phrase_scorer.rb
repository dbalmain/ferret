module Ferret::Search
  class SloppyPhraseScorer < PhraseScorer 

    def initialize(weight, tps, positions, similarity, slop, norms) 
      super(weight, tps, positions, similarity, norms)
      @slop = slop
    end

    def phrase_freq()
      @pq.clear()
      last_pos = 0
      each do |pp|
        pp.first_position()
        last_pos = pp.position if (pp.position > last_pos)
        @pq.push(pp)         # build pq from list
      end

      freq = 0.0
      done = false
      begin 
        pp = @pq.pop()
        pos = start = pp.position
        next_pos = pq.top().position
        while pos <= next_pos
          start = pos       # advance pp to min window
          if not pp.next_position()
            done = true     # ran out of a term -- done
            break
          end
          pos = pp.position
        end

        match_length = last_pos - start
        if (match_length <= slop)
          freq += @similarity.sloppy_freq(match_length) # score match
        end

        if (pp.position > last_pos)
          last_pos = pp.position
        end
        @pq.push(pp)         # restore pq
      end while (!done)

      return freq
    end
  end
end
