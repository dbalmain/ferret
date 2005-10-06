module Ferret::Search
  # A Scorer for queries with a required subscorer and an excluding (prohibited)
  # subscorer.
  #
  # This +Scorer+ implements Scorer#skip_to(int), and it uses the skip_to() on
  # the given scorers.
  class ReqExclScorer < Scorer 
    # Construct a +ReqExclScorer+.
    # req_scorer:: The scorer that must match, except where
    # excl_scorer:: indicates exclusion.
    def initialize(req_scorer, excl_scorer) 
      super(nil) # No similarity used.
      @req_scorer = req_scorer
      @excl_scorer = excl_scorer

      @first_time = true
    end

    
    def next?
      if @first_time
        if not @excl_scorer.next?
          @excl_scorer = nil # exhausted at start
        end
        @first_time = false
      end
      if @req_scorer == nil 
        return false
      end
      if not @req_scorer.next?
        @req_scorer = nil; # exhausted, nothing left
        return false
      end
      if @excl_scorer == nil
        return true # @req_scorer.next? already returned true
      end
      return to_non_excluded()
    end
    
    # Advance to non excluded doc.
    # On entry:
    # 
    # * @req_scorer != nil
    # * @excl_scorer != nil
    # * @req_scorer was advanced once via next? or skip_to() and
    #   @req_scorer.doc() may still be excluded.
    # 
    # Advances @req_scorer a non excluded required doc, if any.
    #
    # returns:: true iff there is a non excluded required doc.
    def to_non_excluded()
      excl_doc = @excl_scorer.doc
      begin 
        req_doc = @req_scorer.doc # may be excluded
        if (req_doc < excl_doc) 
          return true # @req_scorer advanced to before @excl_scorer, ie. not excluded
        elsif (req_doc > excl_doc) 
          unless @excl_scorer.skip_to(req_doc) 
            @excl_scorer = nil # exhausted, no more exclusions
            return true
          end
          excl_doc = @excl_scorer.doc
          if excl_doc > req_doc 
            return true; # not excluded
          end
        end
      end while @req_scorer.next?
      @req_scorer = nil; # exhausted, nothing left
      return false
    end

    # @req_scorer may be nil when next? or skip_to() already return false so
    # only call when you know that a doc exists
    def doc() 
      return @req_scorer.doc
    end

    # Returns the score of the current document matching the query.
    #
    # Initially invalid, until #next? is called the first time.
    #
    # returns:: The score of the required scorer.
    def score()
      return @req_scorer.score()
    end
    
    # Skips to the first match beyond the current whose document number is
    # greater than or equal to a given target.
    #
    # When this method is used the #explain(int) method should not be used.
    #
    # target:: The target document number.
    # returns:: true iff there is such a match.
    def skip_to(target)
      if (@first_time) 
        @first_time = false
        if (! @excl_scorer.skip_to(target)) 
          @excl_scorer = nil; # exhausted
        end
      end
      if (@req_scorer == nil) 
        return false
      end
      if (@excl_scorer == nil) 
        return @req_scorer.skip_to(target)
      end
      if (! @req_scorer.skip_to(target)) 
        @req_scorer = nil
        return false
      end
      return to_non_excluded()
    end

    def explain(doc)
      e = Explanation.new()
      if @excl_scorer.skip_to(doc) and @excl_scorer.doc == doc
        e.description = "excluded"
      else 
        e.description = "not excluded"
        e.details << @req_scorer.explain(doc)
      end
      return e
    end
  end
end
