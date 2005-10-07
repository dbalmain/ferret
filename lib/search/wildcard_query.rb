module Ferret::Search
  # Implements the wildcard search query. Supported wildcards are +*+, which
  # matches any character sequence (including the empty one), and +?+,
  # which matches any single character. Note this query can be slow, as it
  # needs to iterate over many terms. In order to prevent extremely slow WildcardQueries,
  # a Wildcard term should not start with one of the wildcards +*+ or
  # +?+.
  # 
  # See WildcardTermEnum
  class WildcardQuery < MultiTermQuery 
    def initialize(term) 
      super(term)
    end

    def get_term_enum(reader)
      return WildcardTermEnum.new(reader, @term)
    end

    def eql?(o) 
      if o.instance_of?(WildcardQuery)
        return super(o)
      end
      return false
    end
  end
end
