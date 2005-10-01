require 'set'
module Ferret::Search
  # Scorer for conjunctions, sets of queries, all of which are required. 
  class ConjunctionScorer < Scorer 

    def initialize(similarity) 
      super
      @scorers = []
      @first_time = true
      @more = true
    end

    def add(scorer) 
      @scorers << scorer
    end
    alias :<< :add

    def first()
      return @scorers.first
    end

    def last()
      return @scorers.last
    end

    def doc()
      return first().doc()
    end

    def next?()
      if (@first_time) 
        init(true)
      elsif (@more) 
        @more = last().next? # trigger further scanning
      end
      return do_next()
    end
    
    def do_next()
      while @more and first().doc < last().doc # find doc w/ all clauses
        @more = first().skip_to(last().doc)    # skip first upto last
        @scorers << @scorers.shift             # move first to last
      end
      return @more # found a doc with all clauses
    end

    def skip_to(target)
      if(@first_time) 
        init(false)
      end
      
      @scorers.each do |scorer|
        break if not @more
        @more = scorer.skip_to(target)
      end
      
      sort_scorers() if @more # resort the scorers
      
      return do_next()
    end

    # Sums the scores of all of the scorers for the current document.
    def score()
      score = 0.0 # sum scores
      @scorers.each do |scorer|
        score += scorer.score
      end
      score *= @coord
      return score
    end
    
    def init(init_scorers)
      #  compute coord factor
      @coord = similarity().coord(@scorers.size(), @scorers.size())
     
      @more = @scorers.size() > 0

      if init_scorers
        # move each scorer to its first entry
        @scorers.each do |scorer|
          break if not @more
          @more = scorer.next?
        end
        sort_scorers() if @more
      end

      @first_time = false
    end

    def sort_scorers() 
      # move @scorers to an array
      @scorers.sort! {|a,b| a.doc <=> b.doc }
    end

    def explain(doc) 
      raise NotImplementedError
    end
  end
end
