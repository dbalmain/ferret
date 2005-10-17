module Ferret::Search
  # A query that applies a filter to the results of another query.
  # 
  # Note: the bits are retrieved from the filter each time this
  # query is used in a search - use a CachingWrapperFilter to avoid
  # regenerating the bits every time.
  class FilteredQuery < Query 
    attr_accessor :sub_query
    attr_reader :filter

    # Constructs a new query which applies a filter to the results of the
    # original query.
    #
    # Filter.bits() will be called every time this query is used in a search.
    #
    # query::  Query to be filtered, cannot be +nil+.
    # filter:: Filter to apply to query results, cannot be +nil+.
    def initialize(query, filter) 
      super()
      @sub_query = query
      @filter = filter
    end

    # Returns a Weight that applies the filter to the enclosed query's Weight.
    # This is accomplished by overriding the Scorer returned by the Weight.
    def create_weight(searcher)
      sub_weight = @sub_query.create_weight(searcher)
      similarity = @sub_query.similarity(searcher)
      return FilteredWeight.new(self, sub_weight, similarity) 
    end

    class FilteredScorer < Scorer
      def initialize(sub_scorer, bits, similarity)
        super(similarity)
        @sub_scorer = sub_scorer
        @bits = bits
      end
 
      # pass these methods through to the enclosed scorer
      def next?() return @sub_scorer.next?; end
      def doc() return @sub_scorer.doc; end
      def skip_to(i) return @sub_scorer.skip_to(i); end

      # if the document has been filtered out, set score to 0.0
      def score()
        return (@bits.get(@sub_scorer.doc) ? @sub_scorer.score() : 0.0)
      end

      # add an explanation about whether the document was filtered
      def explain(i)
        exp = @sub_scorer.explain(i)
        if (@bits.get(i))
          exp.description = "allowed by filter: #{exp.description}"
        else
          exp.description = "removed by filter: #{exp.description}"
        end
        return exp
      end
    end

    class FilteredWeight < Weight
      attr_reader :query

      def initialize(query, sub_weight, similarity)
        @query = query
        @sub_weight = sub_weight
        @similarity = similarity
      end

      # pass these methods through to enclosed query's weight
      def value()
        return @sub_weight.value
      end

      def sum_of_squared_weights()
        return @sub_weight.sum_of_squared_weights
      end

      def normalize(v)
        return @sub_weight.normalize(v)
      end

      def explain(ir, i)
        return @sub_weight.explain(ir, i)
      end

      # return a scorer that overrides the enclosed query's score if
      # the given hit has been filtered out.
      def scorer(reader)
        scorer = @sub_weight.scorer(reader)
        bits = @query.filter.bits(reader)
        return FilteredScorer.new(scorer, bits, @similarity) 
      end
    end

    # Rewrites the wrapped query. 
    def rewrite(reader)
      rewritten = @sub_query.rewrite(reader)
      if (rewritten != @sub_query) 
        clone = self.clone()
        clone.query = rewritten
        return clone
      else 
        return self
      end
    end

    # inherit javadoc
    def extract_terms(terms) 
      @sub_query.extract_terms(terms)
    end

    # Prints a user-readable version of this query. 
    def to_s(f = nil) 
      return "filtered(#{@sub_query.to_s(f)})->#{@filter}"
    end

    # Returns true iff +o+ is equal to this. 
    def eql?(o) 
      return (o.instance_of?(FilteredQuery) and 
        (@sub_query == o.sub_query) and (@filter == o.filter))
    end
    alias :== :eql?

    # Returns a hash code value for this object. 
    def hash() 
      return @sub_query.hash ^ @filter.hash
    end
  end
end
