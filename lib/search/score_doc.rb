module Ferret::Search
  # Expert: Returned by low-level search implementations.
  # See TopDocs 
  class ScoreDoc
    include Comparable
    # Expert: The score of this document for the query. 
    attr_accessor :score

    # Expert: A hit document's number.
    attr_accessor :doc

    # Expert: Constructs a ScoreDoc. 
    def initialize(doc, score) 
      @doc = doc
      @score = score
    end

    # returns a hash value for storage in a Hash
    def hash()
      return 100 * doc * score
    end

    # score_docA < score_docB if score_docA.score < score_docB.score or
    # score_docA.doc > score_docB.doc
    def <=>(other)
      result = @score.<=>(other.score)
      if (result == 0)
        return other.doc.<=>(@doc)
      else
        return result
      end
    end

    def to_s
      "#{doc}:#{score}"
    end
  end
end
