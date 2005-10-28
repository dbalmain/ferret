module Ferret::Search
  require 'monitor'
  # Constrains search results to only match those which also match a provided
  # query.  Results are cached, so that searches after the first on the same
  # index using this filter are much faster.
  # 
  # This could be used, for example, with a RangeQuery on a suitably formatted
  # date field to implement date filtering.  One could re-use a single
  # QueryFilter that matches, e.g., only documents modified within the last
  # week.  The QueryFilter and RangeQuery would only need to be reconstructed
  # once per day.
  class QueryFilter < Filter 

    # Constructs a filter which only matches documents matching
    # +query+.
    def initialize(query) 
      @query = query
      @cache = nil 
    end

    def bits(reader)

      if (@cache == nil) 
        @cache = Ferret::Utils::WeakKeyHash.new
      end

      @cache.synchronize() do # check cache
        bits = @cache[reader]
        if bits
          return bits
        end
      end

      bits = Ferret::Utils::BitVector.new()

      IndexSearcher.new(reader).search_each(@query) do |doc, score|
        bits.set(doc)  # set bit for hit
      end

      @cache.synchronize() do # update cache
        @cache[reader] = bits
      end

      return bits
    end

    def to_s() 
      return "QueryFilter(#{@query})"
    end
  end
end
