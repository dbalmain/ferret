module Ferret::Search
  # A Scorer for OR like queries, counterpart of Lucene's +ConjunctionScorer+.
  # This Scorer implements Scorer#skip_to(int) and uses skip_to() on the given Scorers. 
  class DisjunctionSumScorer < Scorer 
    # the sub-scorers
    attr_accessor :sub_scorers
    
    # Construct a +DisjunctionScorer+.
    # sub_scorers:: A collection of at least two subscorers.
    #
    # minimum_nr_matchers:: The positive minimum number of subscorers that should
    # match to match this query.
    # <br>When +@minimum_nr_matchers+ is bigger than
    # the number of +sub_scorers+,
    # no matches will be produced.
    # <br>When @minimum_nr_matchers equals the number of sub_scorers,
    # it more efficient to use +ConjunctionScorer+.
    def initialize(sub_scorers, minimum_nr_matchers = 1) 
      super(nil)
      
      # The number of subscorers.  
      @nr_scorers = sub_scorers.size

      # The document number of the current match. 
      @current_doc = -1
      @curret_score = nil
      # The number of subscorers that provide the current match. 
      @nr_matchers = -1

      if (minimum_nr_matchers <= 0) 
        raise ArgumentError, "Minimum nr of matchers must be positive"
      end
      if (@nr_scorers <= 1) 
        raise ArgumentError, "There must be at least 2 sub_scorers"
      end

      @minimum_nr_matchers = minimum_nr_matchers
      @sub_scorers = sub_scorers

      # The @scorer_queue contains all subscorers ordered by their current
      # doc, with the minimum at the top.
      # 
      # The @scorer_queue is initialized the first time next? or skip_to() is
      # called.
      # 
      # An exhausted scorer is immediately removed from the @scorer_queue.
      # 
      # If less than the @minimum_nr_matchers scorers remain in the
      # @scorer_queue next? and skip_to() return false.
      # 
      # After each to call to next? or skip_to()
      # +currentSumScore+ is the total score of the current matching doc,
      # +@nr_matchers+ is the number of matching scorers,
      # and all scorers are after the matching doc, or are exhausted.
      @scorer_queue = nil
    end
    
    # Called the first time next? or skip_to() is called to
    # initialize +@scorer_queue+.
    def init_scorer_queue()
      @scorer_queue = ScorerQueue.new(@nr_scorers)
      @sub_scorers.each do |sub_scorer|
        if (sub_scorer.next?) # doc() method will be used in @scorer_queue.
          @scorer_queue.insert(sub_scorer)
        end
      end
    end

    # A +PriorityQueue+ that orders by Scorer#doc(). 
    class ScorerQueue < Ferret::Utils::PriorityQueue 
      def less_than(scorer1, scorer2) 
        return scorer1.doc < scorer2.doc
      end
    end
    
    def next?
      if (@scorer_queue == nil) 
        init_scorer_queue()
      end

      if (@scorer_queue.size < @minimum_nr_matchers) 
        return false
      else 
        return advance_after_current()
      end
    end


    # Advance all subscorers after the current document determined by the
    # top of the +@scorer_queue+.
    # Repeat until at least the minimum number of subscorers match on the same
    # document and all subscorers are after that document or are exhausted.
    # 
    # On enbegin the +@scorer_queue+ has at least +@minimum_nr_matchers+
    # available. At least the scorer with the minimum document number will be advanced.
    # returns:: true iff there is a match.
    # 
    # In case there is a match, +@current_doc+, +currentSumScore+,
    # and +@nr_matchers+ describe the match.
    # 
    # TODO Investigate whether it is possible to use skip_to() when
    # the minimum number of matchers is bigger than one, ie. begin and use the
    # character of ConjunctionScorer for the minimum number of matchers.
    def advance_after_current()
      begin # repeat until minimum nr of matchers
        top = @scorer_queue.top
        @current_doc = top.doc
        @current_score = top.score
        @nr_matchers = 1
        begin # Until all subscorers are after @current_doc
          if top.next?
            @scorer_queue.adjust_top()
          else 
            @scorer_queue.pop()
            if (@scorer_queue.size < (@minimum_nr_matchers - @nr_matchers)) 
              # Not enough subscorers left for a match on this document,
              # and also no more chance of any further match.
              return false
            end
            if (@scorer_queue.size == 0) 
              break # nothing more to advance, check for last match.
            end
          end
          top = @scorer_queue.top
          if top.doc != @current_doc
            break # All remaining subscorers are after @current_doc.
          else 
            @current_score += top.score
            @nr_matchers += 1
          end
        end while (true)
        
        if (@nr_matchers >= @minimum_nr_matchers) 
          return true
        elsif (@scorer_queue.size < @minimum_nr_matchers) 
          return false
        end
      end while (true)
    end
    
    # Returns the score of the current document matching the query.
    # Initially invalid, until #next? is called the first time.
    def score()
      return @current_score
    end
     
    # Returns the document number of the current document matching the query.
    # Initially invalid, until #next? is called the first time.
    def doc()
      return @current_doc
    end

    # Returns the number of subscorers matching the current document.
    # Initially invalid, until #next? is called the first time.
    def number_of_matchers()
      return @nr_matchers
    end

    # Skips to the first match beyond the current whose document number is
    # greater than or equal to a given target.
    # 
    # When this method is used the #explain(int) method should not be used.
    # 
    # The implementation uses the skip_to() method on the subscorers.
    # target:: The target document number.
    # returns:: true iff there is such a match.
    def skip_to(target)
      if @scorer_queue.nil?
        init_scorer_queue()
      end
      if @scorer_queue.size < @minimum_nr_matchers
        return false
      end
      if target <= @current_doc
        target = @current_doc + 1
      end
      begin 
        top = @scorer_queue.top
        if top.doc >= target 
          return advance_after_current()
        elsif top.skip_to(target) 
          @scorer_queue.adjust_top()
        else 
          @scorer_queue.pop()
          if (@scorer_queue.size < @minimum_nr_matchers) 
            return false
          end
        end
      end while (true)
    end

    # Gives and explanation for the score of a given document.
    # TODO Show the resulting score. See BooleanScorer.explain() on how to do this.
    def explain(doc)
      e = Explanation.new()
      e.description = "At least " + @minimum_nr_matchers + " of"
      @sub_scorers.each do |sub_scorer|
        e.details << sub_scorer.explain(doc)
      end
      return e
    end
  end
end
