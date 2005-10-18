module Ferret::Search::Spans
  # Base class for span-based queries. 
  class SpanQuery < Ferret::Search::Query 
    # Expert: Returns the matches for this query in an index.  Used internally
    # to search for spans. 
    def spans(reader)
      raise NotImplementedError
    end

    # Returns the name of the field matched by this query.
    def field()
      raise NotImplementedError
    end

    # Returns a collection of all terms matched by this query.
    def terms()
      raise NotImplementedError
    end

    def create_weight(searcher)
      return SpanWeight.new(self, searcher)
    end
  end
end

