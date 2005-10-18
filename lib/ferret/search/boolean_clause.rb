
module Ferret::Search

  # A clause in a BooleanQuery. 
  class BooleanClause
    
    class Occur < Ferret::Utils::Parameter
      
      def to_s() 
        return "+" if (self == MUST)
        return "-" if (self == MUST_NOT)
        return ""
      end

      # Use this operator for terms that _must_ appear in the matching
      # documents. 
      MUST = Occur.new("MUST")

      # Use this operator for terms that _should_ appear in the matching
      # documents. For a BooleanQuery with two +SHOULD+ subqueries, at
      # least one of the queries must appear in the matching documents. 
      SHOULD = Occur.new("SHOULD")

      # Use this operator for terms that _must not_ appear in the matching
      # documents.  Note that it is not possible to search for queries that
      # only consist of a +MUST_NOT+ query. 
      MUST_NOT = Occur.new("MUST_NOT")
    end

    # The query whose matching documents are combined by the boolean query.
    attr_accessor :query

    # If true, documents documents which _do not_ match this sub-query will
    # _not_ match the boolean query.
    attr_writer :required
    def required?
      @required
    end
    
    # If true, documents documents which _do_ match this sub-query will _not_
    # match the boolean query.
    attr_writer :prohibited
    def prohibited?
      @prohibited
    end

    # See BooleanQuery::Occur for values for this attribute
    attr_reader :occur
    def occur=(occur) 
      @occur = occur
      set_fields(occur)
    end

    # Constructs a BooleanClause. Default value for occur is Occur::SHOULD
    def initialize(query, occur = Occur::SHOULD) 
      @query = query
      @occur = occur
      set_fields(occur)
    end


    # Returns true iff +other+ is equal to this. 
    def eql?(other) 
      if not other.instance_of?(BooleanClause)
        return false
      end
      return (@query == other.query and
              @required == other.required? and
              @prohibited == other.prohibited?)
    end
    alias :== :eql?

    # Returns a hash code value for this object.
    def hash() 
      return @query.hash() ^ (@required ? 1 : 0) ^ (@prohibited ? 2 : 0)
    end

    # represent a boolean clause as a string
    def to_s() 
      return @occur.to_s() + @query.to_s()
    end
    
    private

      def set_fields(occur) 
        if (occur == Occur::MUST) 
          @required = true
          @prohibited = false
        elsif (occur == Occur::SHOULD) 
          @required = false
          @prohibited = false
        elsif (occur == Occur::MUST_NOT) 
          @required = false
          @prohibited = true
        else 
          raise ArgumentError, "Unknown operator " + occur
        end
      end
  end
end
