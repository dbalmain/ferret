module Ferret::Search
  # A Query that matches documents containing a subset of terms provided
  # by a FilteredTermEnum enumeration.
  #
  # +MultiTermQuery+ is not designed to be used by itself. The reason being
  # that it is not intialized with a FilteredTermEnum enumeration. A
  # FilteredTermEnum enumeration needs to be provided.
  #
  # For example, WildcardQuery and FuzzyQuery extend +MultiTermQuery+ to
  # provide WildcardTermEnum and FuzzyTermEnum, respectively.
  class MultiTermQuery < Query 
    attr_reader :term

    # Constructs a query for terms matching +term+. 
    def initialize(term) 
      super()
      @term = term
    end

    # Construct the enumeration to be used, expanding the pattern term. 
    def get_term_enum(reader)
      raise NotImplementedError
    end
     

    def rewrite(reader)
      enumerator = get_term_enum(reader)
      bq = BooleanQuery.new(true)
      begin 
        begin 
          t = enumerator.term()
          if (t != nil)
            tq = TermQuery.new(t)      # found a match
            tq.boost = boost() * enumerator.difference()   # set the boost
            bq.add_query(tq, BooleanClause::Occur::SHOULD) # add to query
          end
        end while enumerator.next?
      ensure 
        enumerator.close()
      end
      return bq
    end

    # Prints a user-readable version of this query. 
    def to_s(field) 
      buffer = ""
      buffer << "#{@term.field}:" if @term.field != field
      buffer << @term.text
      buffer << "^#{boost()}" if (boost() != 1.0) 
      return buffer
    end

    def eql?(o) 
      if not o.instance_of? MultiTermQuery
        return false
      end
      return term == o.term
    end
    alias :== :eql?

    def hash() 
      return term.hash()
    end
  end
end
