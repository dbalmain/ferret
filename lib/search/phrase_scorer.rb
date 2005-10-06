module Ferret::Search
  class PhraseScorer < Scorer 
    attr_reader :first, :last
    protected   :first, :last

    def initialize(weight, tps, positions, similarity, norms) 
      super(similarity)
      @norms = norms
      @weight = weight
      @value = weight.value
      @first_time = true
      @more = true

      # convert tps to a list
      tps.length.times do |i|
        pp = PhrasePositions.new(tps[i], positions[i])
        if (@last != nil) # add next to end of list
          @last.next = pp
        else
          @first = pp
        end
        @last = pp
      end

      @pq = PhraseQueue.new(tps.length)  # construct empty pq
    end

    def doc()
      return @first.doc
    end

    def next?
      if (@first_time) 
        init()
        @first_time = false
      elsif (@more) 
        @more = @last.next?                      # trigger further scanning
      end
      return do_next()
    end
    
    # next without initial increment
    def do_next()
      while (@more) 
        while (@more and @first.doc < @last.doc) # find doc w/ all the terms
          @more = @first.skip_to(@last.doc)      # skip first upto last
          first_to_last()                        # and move it to the end
        end

        if (@more) 
          # found a doc with all of the terms
          @freq = phrase_freq()                  # check for phrase
          if (@freq == 0.0)                      # no match
            @more = @last.next?                  # trigger further scanning
          else
            return true                          # found a match
          end
        end
      end
      return false                               # no more matches
    end

    def each()
      pp = @first
      while (pp != nil)
        yield pp
        pp = pp.next
      end
    end

    def score()
      #puts("scoring #{@first.doc}")
      raw = similarity().tf(@freq) * @value      # raw score
      return raw * Similarity.decode_norm(@norms[@first.doc])  # normalize
    end

    def skip_to(target)
      each() { |pp| break if not @more = pp.skip_to(target) }
      sort() if @more                            # re-sort
      return do_next()
    end

    def phrase_freq()
      raise NotImplementedError
    end

    def init()
      each do |pp|
        break if not @more = pp.next?
      end
      if @more
        sort()
      end
    end
    
    def sort() 
      @pq.clear()
      each() do |pp|
        @pq.push(pp)
      end
      pq_to_list()
    end

    def pq_to_list() 
      @last = @first = nil
      while (@pq.top() != nil) 
        pp = @pq.pop()
        if (@last != nil) # add next to end of list
          @last.next = pp
        else
          @first = pp
        end
        @last = pp
        pp.next = nil
      end
    end

    def first_to_last() 
      @last.next = @first  # move first to end of list
      @last = @first
      @first = @first.next
      @last.next = nil
    end

    def explain(doc)
      tf_explanation = Explanation.new()

      while (next? and doc() < doc)
      end

      phrase_freq = (doc() == doc) ? @freq : 0.0
      tf_explanation.value = @similarity.tf(phrase_freq)
      tf_explanation.description = "tf(phrase_freq=#{phrase_freq})"

      return tf_explanation
    end

    def to_s() return "phrase_scorer(#{@weight})" end

  end


  class PhraseQueue < PriorityQueue
    def less_than(pp1, pp2)
      if (pp1.doc == pp2.doc) 
        return pp1.position < pp2.position
      else
        return pp1.doc < pp2.doc
      end
    end
  end

end
