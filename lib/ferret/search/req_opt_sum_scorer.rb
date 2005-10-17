module Ferret::Search
  # A Scorer for queries with a required part and an optional part.
  # Delays skip_to() on the optional part until a score() is needed.
  # 
  # This +Scorer+ implements Scorer#skip_to(int).
  class ReqOptSumScorer < Scorer 
    # The scorers passed from the constructor.
    # These are set to nil as soon as their next? or skip_to() returns false.
    #
    # Construct a +ReqOptScorer+.
    # req_scorer:: The required scorer. This must match.
    # opt_scorer:: The optional scorer. This is used for scoring only.
    def initialize(req_scorer, opt_scorer)
      super(nil) # No similarity used.
      @req_scorer = req_scorer
      @opt_scorer = opt_scorer

      @first_time_opt_scorer = true
    end


    def next?
      return @req_scorer.next?
    end

    def skip_to(target)
      return @req_scorer.skip_to(target)
    end

    def doc() 
      return @req_scorer.doc()
    end

    # Returns the score of the current document matching the query.
    # Initially invalid, until #next? is called the first time.
    #
    # returns:: The score of the required scorer, eventually increased by the
    #           score of the optional scorer when it also matches the current
    #           document.
    def score()
      cur_doc = @req_scorer.doc
      req_score = @req_scorer.score
      if @first_time_opt_scorer
        @first_time_opt_scorer = false
        if not @opt_scorer.skip_to(cur_doc) 
          @opt_scorer = nil
          return req_score
        end
      elsif @opt_scorer.nil?
        return req_score
      elsif @opt_scorer.doc < cur_doc and not @opt_scorer.skip_to(cur_doc)
        @opt_scorer = nil
        return req_score
      end
      # assert (@opt_scorer != nil) and (@opt_scorer.doc() >= cur_doc)
      return (@opt_scorer.doc == cur_doc) ? req_score + @opt_scorer.score() : req_score
    end

    # Explain the score of a document.
    # @todo Also show the total score.
    # See BooleanScorer.explain() on how to do this.
    def explain(doc)
      e = Explanation.new()
      e.description = "required, optional"
      e.details << @req_scorer.explain(doc)
      e.details << @opt_scorer.explain(doc)
      return e
    end
  end
end
