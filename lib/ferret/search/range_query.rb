module Ferret::Search
  # A Query that matches documents within an exclusive range. A RangeQuery
  # is built by QueryParser for input like +[010 120]+.
  class RangeQuery < Query
    attr_reader :lower_term, :upper_term

    # Constructs a query selecting all terms greater than
    # +lower_term+ but less than +upper_term+.
    # There must be at least one term and either term may be nil,
    # in which case there is no bound on that side, but if there are
    # two terms, both terms *must* be for the same field.
    #
    # field:: The field this range applies to
    # lower_term:: The lower bound on this range
    # upper_term:: The upper bound on this range
    # include_lower:: Does this range include the lower bound?
    # include_upper:: Does this range include the upper bound?
    def initialize(field, lower_term, upper_term, include_lower, include_upper) 
      super()
      @field = field
      @lower_term = lower_term
      @upper_term = upper_term
      @include_lower = include_lower
      @include_upper = include_upper
      
      if (lower_term.nil? and upper_term.nil?) 
        raise ArgumentError, "At least one value must be non-nil"
      end
      if (include_lower and lower_term.nil?) 
        raise ArgumentError, "The lower bound must be non-nil to be inclusive"
      end
      if (include_upper and upper_term.nil?) 
        raise ArgumentError, "The upper bound must be non-nil to be inclusive"
      end
      if (upper_term and lower_term and upper_term < lower_term)
        raise ArgumentError, "The lower bound must less than the upper bound"
      end
    end
    
    # Constructs a query for field +field+ matching less than or equal to
    # +upper_term+.
    def RangeQuery.new_less(field, upper_term, include_upper = true) 
      return RangeQuery.new(field, nil, upper_term, false, include_upper)
    end

    # Constructs a query for field +field+ matching greater than or equal
    # to +lower_term+.
    def RangeQuery.new_more(field, lower_term, include_lower = true) 
      return RangeQuery.new(field, lower_term, nil, include_lower, false)
    end

    def rewrite(reader)
      bq = BooleanQuery.new(true)
      term_enum = reader.terms_from(Term.new(@field, @lower_term||""))

      begin 
        check_lower = !@include_lower
        test_field = field()
        begin 
          term = term_enum.term

          break if term.nil? or term.field != @field         
          if (!check_lower or @lower_term.nil? or term.text > @lower_term) 
            check_lower = false
            if @upper_term
              compare = @upper_term <=> term.text

              # if beyond the upper term, or is exclusive and
              # this is equal to the upper term, break out 
              if ((compare < 0) or (not @include_upper and compare == 0))
                break
              end
            end
            tq = TermQuery.new(term) # found a match
            tq.boost = boost()       # set the boost
            bq.add_query(tq, BooleanClause::Occur::SHOULD) # add to query
          end
        end while term_enum.next?
      ensure 
        term_enum.close()
      end
      return bq
    end

    # Returns the field name for this query 
    attr_reader :field, :lower_term, :upper_term, :include_lower, :include_upper

    # Prints a user-readable version of this query. 
    def to_s(f=nil)
      buffer = ""
      buffer << "#{@field}:" if field() != f

      if @lower_term
        buffer << (@include_lower ? "[" : "{")
        buffer << @lower_term
      else
        buffer << "|"
      end

      buffer << " " if @upper_term and @lower_term

      if @upper_term
        buffer << @upper_term
        buffer << (@include_upper ? "]" : "}")
      else
        buffer << "|"
      end

      if boost() != 1.0
        buffer << "^#{boost()}"
      end
      return buffer 
    end

    # Returns true iff +o+ is equal to this. 
    def eql?(o) 
      return ((o.instance_of?(RangeQuery)) and 
          (boost() == o.boost()) and
          (@include_upper == o.include_upper) and
          (@include_lower == o.include_lower) and
          (@upper_term == o.upper_term) and
          (@lower_term == o.lower_term) and
          (@field == o.field))
    end
    alias :== :eql?

    # Returns a hash code value for this object.
    def hash() 
      return (boost().hash ^
              @field.hash ^
              @lower_term.hash ^
              @upper_term.hash ^
              @include_lower.hash ^
              @include_upper.hash)
    end
  end
end
