module Ferret::Search
  # Expert: Returned by low-level search implementations.
  # See Searcher#search
  class TopDocs
    # Expert: The total number of hits for the query.
    # See Hits#length()
    attr_accessor :score_docs, :total_hits

    # Expert: Constructs a TopDocs.
    def initialize(total_hits, score_docs) 
      @total_hits = total_hits
      @score_docs = score_docs
    end
  end
end
