module Ferret::Search
  # Expert: Returned by low-level sorted search implementations.
  class TopFieldDocs < TopDocs 

    # The fields which were used to sort results by. 
    attr_accessor :fields

    # Creates one of these objects.
    # total_hits::  Total number of hits for the query.
    # score_docs::  The top hits for the query.
    # fields::     The sort criteria used to find the top hits.
    def initialize(total_hits, score_docs, fields) 
      super(total_hits, score_docs)
      @fields = fields
    end
  end
end
