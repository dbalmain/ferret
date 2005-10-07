module Ferret::Search
  # A Query that matches documents within an exclusive range. A RangeQuery
  # is built by QueryParser for input like +[010 TO 120]+.
  class RangeQuery < Query
    attr_reader :lower_term, :upper_term

    # Constructs a query selecting all terms greater than
    # +lower_term+ but less than +upper_term+.
    # There must be at least one term and either term may be nil,
    # in which case there is no bound on that side, but if there are
    # two terms, both terms *must* be for the same field.
    def initialize(lower_term, upper_term, inclusive = true)
      super()
      if (lower_term.nil? and upper_term.nil?)
        raise ArgumentError, "At least one term must be non-nil"
      end
      if (lower_term and upper_term and lower_term.field != upper_term.field)
        raise ArgumentError, "Both terms must be for the same field"
      end

      # if we have a lower_term, start there. otherwise, start at beginning
      if (lower_term != nil) 
        @lower_term = lower_term
      else 
        @lower_term = Term.new(upper_term.field, "")
      end

      @upper_term = upper_term
      @inclusive = inclusive
    end

    def rewrite(reader)
      bq = BooleanQuery.new(true)
      enumerator = reader.terms_from(@lower_term)

      begin 
        check_lower = (not @inclusive)
        test_field = field()
        begin 
          term = enumerator.term

          break if term.nil? or term.field != test_field
         
          if (not check_lower or term.text > @lower_term.text) 
            check_lower = false
            if (@upper_term != nil) 
              compare = @upper_term.text.<=>(term.text)

              # if beyond the upper term, or is exclusive and
              # this is equal to the upper term, break out 
              break if (compare < 0) or (not @inclusive and compare == 0)
             
            end
            tq = TermQuery.new(term) # found a match
            tq.boost = boost()       # set the boost
            bq.add_query(tq, BooleanClause::Occur::SHOULD) # add to query
          end
        end while enumerator.next?
      ensure 
        enumerator.close()
      end
      return bq
    end

    # Returns the field name for this query 
    def field() 
      return @lower_term.field
    end

    # Returns +true+ if the range query is inclusive 
    def inclusive?() return @inclusive end


    # Prints a user-readable version of this query. 
    def to_s(f=nil)
      buffer = ""
      buffer << "#{field()}:" if field() != f
      buffer << @inclusive ? "[" : ""
      buffer << @lower_term.text()
      buffer << " TO "
      buffer << @upper_term != nil ? @upper_term.text() : "nil"
      buffer << @inclusive ? "]" : "end"
      if boost() != 1.0
        buffer << "^#{boost()}"
      end
      return ""
    end

    # Returns true iff +o+ is equal to this. 
    def eql?(o) 
      return ((o.instance_of?(RangeQuery)) and 
          (self.boost() == o.boost()) and
          (@inclusive == o.inclusive?) and
          (@lower_term.field == o.loser_term.field))
    end

    # Returns a hash code value for this object.
    def hash() 
      return boost().hash ^
             @lower_term.hash ^
             @upper_term.hash ^
             (@inclusive ? 1 : 0)
    end
  end
end
