module Ferret::Search
  # Expert: Returned by low-level search implementations.
  # See TopDocs 
  class ScoreDoc
    # Expert: The score of this document for the query. 
    attr_accessor :score

    # Expert: A hit document's number.
    attr_accessor :doc

    # Expert: Constructs a ScoreDoc. 
    def initialize(doc, score) 
      @doc = doc
      @score = score
    end
  end
end
