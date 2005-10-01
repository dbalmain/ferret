module Ferret::Search
  # Abstract base class providing a mechanism to restrict searches to a subset
  # of an index. 
  class Filter
    # Returns a BitSet with true for documents which should be permitted in
    # search results, and false for those that should not. 
    def bits(reader)
      raise NotImplementedError
    end
  end
end
