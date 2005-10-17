module Ferret::Search
  # Expert: Common scoring functionality for different types of queries.
  # 
  # A +Scorer+ either iterates over documents matching a query, or provides an
  # explanation of the score for a query for a given document.
  # 
  # Document scores are computed using a given +Similarity+ implementation.
  class Scorer 
    attr_reader :similarity
    MAX_DOCS = 0x7FFFFFFF

    # Constructs a Scorer.
    # similarity:: The +Similarity+ implementation used by this scorer.
    def initialize(similarity) 
      @similarity = similarity
    end

    # Expert: Iterates over matching all documents, yielding the document
    # number and the score.
    #
    # returns:: true if more matching documents may remain.
    def each_hit() # :yields: doc, score
      while next?
        yield(doc(), score())
      end
    end
 
    # Expert: Iterates over matching documents in a range.
    #
    # max:: Do not score documents past this. Default will search all documents
    # avaliable.
    # returns:: true if more matching documents may remain.
    def each_hit_up_to(max = MAX_DOCS) # :yields: doc, score
      while (next? and doc() < max)
        yield(doc(), score())
      end
      return doc() < max
    end

    # Advances to the next document matching the query.
    # returns:: true iff there is another document matching the query.
    # When this method is used the #explain(int) method should not be used.
    def next?()
      raise NotImplementedError
    end

    # Returns the current document number matching the query.
    # Initially invalid, until #next?() is called the first time.
    def doc()
      raise NotImplementedError
    end

    # Returns the score for the current document matching the query.
    # Initially invalid, until #next?() is called the first time.
    def score()
      raise NotImplementedError
    end

    # Skips to the first match beyond the current whose document number is
    # greater than or equal to a given target.
    #
    # When this method is used the #explain(int) method should not be used.
    #
    # target:: The target document number.
    # returns:: true iff there is such a match.
    #
    # Behaves as if written:
    #
    #   def skip_to(target) 
    #     begin 
    #       return false if not next?()
    #     end while (target > doc())
    #     return true
    #   end
    #
    # Most implementations are considerably more efficient than that.
    def skip_to(target)
      raise NotImplementedError
    end

    # Returns an explanation of the score for a document.
    #
    # When this method is used, the #next?(), #skip_to(int) and
    # #score(HitCollector) methods should not be used.
    #
    # doc:: The document number for the explanation.
    def explain(doc)
      raise NotImplementedError
    end
  end
end
