module Ferret::Search
  # A Filter that restricts search results to a range of values in a given
  # field.
  # 
  # This code borrows heavily from RangeQuery, but is implemented as a Filter.
  class RangeFilter < Filter 
    # field_name:: The field this range applies to
    # lower_term:: The lower bound on this range
    # upper_term:: The upper bound on this range
    # include_lower:: Does this range include the lower bound?
    # include_upper:: Does this range include the upper bound?
    def initialize(field_name, lower_term, upper_term, include_lower, include_upper) 
      @field_name = field_name
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
    
    # Constructs a filter for field +field_name+ matching less than or equal to
    # +upper_term+.
    def RangeFilter.new_less(field_name, upper_term, include_upper = true) 
      return RangeFilter.new(field_name, nil, upper_term, false, include_upper)
    end

    # Constructs a filter for field +field_name+ matching greater than or equal
    # to +lower_term+.
    def RangeFilter.new_more(field_name, lower_term, include_lower = true) 
      return RangeFilter.new(field_name, lower_term, nil, include_lower, false)
    end
    
    # Returns a BitVector with true for documents which should be permitted in
    # search results, and false for those that should not.
    def bits(reader)
      bits = BitVector.new()
      term_enum = reader.terms_from(Term.new(@field_name, @lower_term||""))
      
      begin 
        if (term_enum.term() == nil) 
          return bits
        end
        check_lower = !@include_lower # make adjustments to set to exclusive
      
        term_docs = reader.term_docs
        begin 
          begin 
            term = term_enum.term()
            break if (term.nil? or term.field != @field_name) 

            if (!check_lower or @lower_term.nil? or term.text > @lower_term) 
              check_lower = false
              if @upper_term
                compare = @upper_term <=> term.text
                # if beyond the upper term, or is exclusive and
                # this is equal to the upper term, break out 
                if ((compare < 0) or (!@include_upper and compare == 0))
                  break
                end
              end
              # we have a good term, find the docs 
              
              term_docs.seek(term_enum)
              while term_docs.next?
                bits.set(term_docs.doc)
              end
            end
          end while term_enum.next?
        ensure 
          term_docs.close()
        end
      ensure 
        term_enum.close()
      end

      return bits
    end
    
    def to_s() 
      buffer = "#{@field_name}:"
      buffer << "[" if @include_lower
      buffer << @lower_term if @lower_term
      buffer << "-"
      buffer << @upper_term if @upper_term
      buffer << @include_upper ? "]" : "end"
      return buffer
    end
  end
end
