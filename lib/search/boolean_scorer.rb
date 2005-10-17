module Ferret::Search
  # An alternative to BooleanScorer.
  # 
  # Uses ConjunctionScorer, DisjunctionScorer, ReqOptScorer and ReqExclScorer.
  # 
  # Implements skip_to(), and has no limitations on the numbers of added scorers.
  class BooleanScorer < Scorer 
    attr_reader :required_scorers, :coordinator

    class Coordinator 
      attr_accessor :max_coord, :nr_matchers

      def initialize(similarity)
        @max_coord = 0 # to be increased for each non prohibited scorer
        @coord_factors = nil
        @similarity = similarity
      end
      
      
      def init() # use after all scorers have been added.
        @coord_factors = Array.new(@max_coord + 1)

        (@max_coord+1).times do |i|
          @coord_factors[i] = @similarity.coord(i, @max_coord)
        end
      end
      

      def init_doc() 
        @nr_matchers = 0
      end
      
      def coord_factor() 
        return @coord_factors[@nr_matchers]
      end
    end

    # The scorer to which all scoring will be delegated,
    # except for computing and using the coordination factor.

    def initialize(similarity) 
      super(similarity)
      @required_scorers = []
      @optional_scorers = []
      @prohibited_scorers = []
      @counting_sum_scorer = nil
      @coordinator = Coordinator.new(similarity)
    end

    def add_scorer(scorer, occur) 
      unless occur == BooleanClause::Occur::MUST_NOT
        @coordinator.max_coord += 1
      end

      case occur
      when BooleanClause::Occur::MUST: @required_scorers << scorer
      when BooleanClause::Occur::SHOULD: @optional_scorers << scorer
      when BooleanClause::Occur::MUST_NOT: @prohibited_scorers << scorer
      end
    end

    # Initialize the match counting scorer that sums all the
    # scores. 
    # When "counting" is used in a name it means counting the number
    # of matching scorers.<br>
    # When "sum" is used in a name it means score value summing
    # over the matching scorers
    def init_counting_sum_scorer() 
      @coordinator.init()
      @counting_sum_scorer = make_counting_sum_scorer()
    end

    # Count a scorer as a single match. 
    class SingleMatchScorer < Scorer 
      def initialize(parent_scorer, scorer) 
        super(scorer.similarity)
        @scorer = scorer
        @parent_scorer = parent_scorer
      end
      def score()
        @parent_scorer.coordinator.nr_matchers += 1
        return @scorer.score
      end
      def doc() 
        return @scorer.doc
      end
      def next?
        return @scorer.next?
      end
      def skip_to(doc_nr)
        return @scorer.skip_to(doc_nr)
      end
      def explain(doc_nr)
        return @scorer.explain(doc_nr)
      end
    end

    class CountingDisjunctionSumScorer < DisjunctionSumScorer
      def initialize(parent_scorer, scorers)
        super(scorers)
        @parent_scorer = parent_scorer
      end
      def score
        @parent_scorer.coordinator.nr_matchers += @nr_matchers
        return super
      end
    end

    def counting_disjunction_sum_scorer(scorers)
    # each scorer from the list counted as a single matcher
    
      return CountingDisjunctionSumScorer.new(self, scorers) 
    end

    class CountingConjunctionScorer < ConjunctionScorer
      def initialize(parent_scorer, similarity)
        super(similarity)
        @parent_scorer = parent_scorer
        @required_nr_matchers = parent_scorer.required_scorers.size
        @last_scored_doc = -1
      end
      def score
        if (@parent_scorer.doc() > @last_scored_doc)
          @last_scored_doc = @parent_scorer.doc()
          @parent_scorer.coordinator.nr_matchers += @required_nr_matchers
        end

        return super
      end
    end

    def counting_conjunction_sum_scorer(required_scorers)
      # each scorer from the list counted as a single matcher
    
      required_nr_matchers = required_scorers.size
      ccs = CountingConjunctionScorer.new(self, Similarity.default)
      @required_scorers.each do |scorer|
        ccs << scorer
      end
      return ccs
    end

    # Returns the scorer to be used for match counting and score summing.
    # Uses required_scorers, optional_scorers and prohibited_scorers.
    def make_counting_sum_scorer()
    # each scorer counted as a single matcher
      if @required_scorers.size == 0
        if @optional_scorers.size == 0
          return NonMatchingScorer.new # only prohibited scorers
        elsif @optional_scorers.size == 1
          return make_counting_sum_scorer2( # the only optional scorer is required
                    SingleMatchScorer.new(self, @optional_scorers[0]),
                    []) # no optional scorers left
        else # more than 1 @optional_scorers, no required scorers
          return make_counting_sum_scorer2( # at least one optional scorer is required
                    counting_disjunction_sum_scorer(@optional_scorers), 
                    []) # no optional scorers left
        end
      elsif @required_scorers.size == 1 # 1 required
        return make_counting_sum_scorer2(
                    SingleMatchScorer.new(self, @required_scorers[0]),
                    @optional_scorers)
      else # more required scorers
        return make_counting_sum_scorer2(
                    counting_conjunction_sum_scorer(@required_scorers),
                    @optional_scorers)
      end
    end

    # Returns the scorer to be used for match counting and score summing.
    # Uses the arguments and prohibited_scorers.
    # required_counting_sum_scorer:: A required scorer already built.
    # @optional_scorers:: A list of optional scorers, possibly empty.
    def make_counting_sum_scorer2(required_counting_sum_scorer, optional_scorers)
    
      if (optional_scorers.size == 0)
        if (@prohibited_scorers.size == 0)
          return required_counting_sum_scorer
        elsif (@prohibited_scorers.size == 1)
          return ReqExclScorer.new(required_counting_sum_scorer,
                                   @prohibited_scorers[0])
        else # no optional, more than 1 prohibited
          return ReqExclScorer.new(
                        required_counting_sum_scorer,
                        DisjunctionSumScorer.new(@prohibited_scorers))
        end
      elsif (optional_scorers.size == 1)
        return make_counting_sum_scorer3(
                        required_counting_sum_scorer,
                        SingleMatchScorer.new(self, optional_scorers[0]))
      else # more optional
        return make_counting_sum_scorer3(
                        required_counting_sum_scorer,
                        counting_disjunction_sum_scorer(optional_scorers))
      end
    end

    # Returns the scorer to be used for match counting and score summing.
    # Uses the arguments and prohibited_scorers.
    # required_counting_sum_scorer:: A required scorer already built.
    # optional_counting_sum_scorer:: An optional scorer already built.
    def make_counting_sum_scorer3(required_counting_sum_scorer,
                                  optional_counting_sum_scorer)
      if (@prohibited_scorers.size == 0) # no prohibited
        return ReqOptSumScorer.new(required_counting_sum_scorer,
                                   optional_counting_sum_scorer)
      elsif (@prohibited_scorers.size == 1) # 1 prohibited
        return ReqOptSumScorer.new(
                  ReqExclScorer.new(required_counting_sum_scorer,
                                    @prohibited_scorers[0]),
                  optional_counting_sum_scorer)
      else # more prohibited
        return ReqOptSumScorer.new(
                  ReqExclScorer.new(required_counting_sum_scorer,
                                    DisjunctionSumScorer.new(@prohibited_scorers)),
                  optional_counting_sum_scorer)
      end
    end

    # Expert: Iterates over matching all documents, yielding the document
    # number and the score.
    #
    # returns:: true if more matching documents may remain.
    def each_hit() # :yields: doc, score
      if @counting_sum_scorer.nil?
        init_counting_sum_scorer()
      end
      while @counting_sum_scorer.next?
        yield(@counting_sum_scorer.doc, score())
      end
    end

    # Expert: Iterates over matching documents in a range.
    #
    # NOTE: that #next? needs to be called first.
    #
    # max:: Do not score documents past this. Default will search all documents
    # avaliable.
    # returns:: true if more matching documents may remain.
    def each_hit_up_to(max = MAX_DOCS) # :yields: doc, score
      # nil pointer exception when next? was not called before:
      doc_nr = @counting_sum_scorer.doc()
      while (doc_nr < max) 
        yield(doc_nr, score())
        if not @counting_sum_scorer.next? 
          return false
        end
        doc_nr = @counting_sum_scorer.doc()
      end
      return true
    end

    def doc()
      return @counting_sum_scorer.doc
    end

    def next?
      if (@counting_sum_scorer == nil) 
        init_counting_sum_scorer()
      end
      return @counting_sum_scorer.next?
    end

    def score()
      @coordinator.init_doc()
      sum = @counting_sum_scorer.score()
      return sum * @coordinator.coord_factor()
    end

    # Skips to the first match beyond the current whose document number is
    # greater than or equal to a given target.
    # 
    # When this method is used the #explain(int) method should not be used.
    # 
    # target:: The target document number.
    # returns:: true iff there is such a match.
    def skip_to(target)
      if (@counting_sum_scorer == nil) 
        init_counting_sum_scorer()
      end
      return @counting_sum_scorer.skip_to(target)
    end

    # TODO: Implement an explanation of the coordination factor.
    # doc:: The document number for the explanation.
    # raises:: UnsupportedOperationException
    def explain(doc) 
      raise NotImplementedError
      # How to explain the coordination factor?
      #init_counting_sum_scorer()
      #return @counting_sum_scorer.explain(doc); # misses coord factor. 
    end
  end
end
