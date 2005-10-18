module Ferret::Search

  # Abstract class for enumerating a subset of all terms. 
  #
  # Term enumerations are always ordered by Term.<=>().  Each term in
  # the enumeration is greater than all that precede it.  
  class FilteredTermEnum < Ferret::Index::TermEnum 
      
    # Returns the current Term in the enumeration.
    # Returns nil if no Term matches or all terms have been enumerated. 
    attr_reader :term
      
    def initialize()
      @term = nil
      @enum = nil
      @reader = nil
    end

    # Equality compare on the term 
    def term_compare(term)
      raise NotImplementedError
    end
      
    # Equality measure on the term 
    def difference()
      raise NotImplementedError
    end

    # Indiciates the end of the enumeration has been reached 
    def end_enum()
      raise NotImplementedError
    end
      
    def enum=(enum)
      @enum = enum
      # Find the first term that matches
      term = @enum.term()
      if (term != nil and term_compare(term)) 
        @term = term
      else
        next?
      end
    end
      
    # Returns the doc_freq of the current Term in the enumeration.
    # Returns -1 if no Term matches or all terms have been enumerated.
    def doc_freq() 
      if (@enum == nil)
        return -1
      end
      return @enum.doc_freq()
    end
    
    # Increments the enumeration to the next element.  True if one exists. 
    def next?()
      return false if (@enum == nil) # enum not initialized
      @term = nil
      while @term.nil? 
        if end_enum() or ! @enum.next?
          return false
        end
        term = @enum.term()
        if (term_compare(term)) 
          @term = term
          return true
        end
      end
      @term = nil
      return false
    end

    # Closes the enumeration to further activity, freeing resources.  
    def close()
      @enum.close()
      @term = nil
      @enum = nil
    end
  end
end
