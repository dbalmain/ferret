module Ferret
  module Search
    # Expert: Calculate query weights and build query scorers.
    # 
    # The purpose of Weight is to make it so that searching does not modify
    # a Query, so that a Query instance can be reused.
    #
    # Searcher dependent state of the query should reside in the Weight.
    #
    # IndexReader dependent state should reside in the Scorer.
    # 
    # A +Weight+ is used in the following way:
    # 
    # 1. A +Weight+ is constructed by a top-level query, given a +Searcher+
    #    (See Query#create_weight).
    # 2. The #sum_of_squared_weights() method is called on the +Weight+ to
    #    compute the query normalization factor Similarity#query_norm(float)
    #    of the query clauses contained in the query.
    # 3. The query normalization factor is passed to #normalize().
    #    At this point the weighting is complete.
    # 4. A +Scorer+ is constructed by #scorer()
    class Weight
      # The query that this concerns. 
      def query()
        raise NotImplementedError
      end

      # The weight for this query. 
      def  value()
        raise NotImplementedError
      end

      # The sum of squared weights of contained query clauses. 
      def sum_of_squared_weights()
        raise NotImplementedError
      end

      # Assigns the query normalization factor to this. 
      def normalize(norm)
        raise NotImplementedError
      end

      # Constructs a scorer for this. 
      def scorer(reader)
        raise NotImplementedError
      end

      # An explanation of the score computation for the named document. 
      def explain(reader, doc)
        raise NotImplementedError
      end
    end
  end
end
