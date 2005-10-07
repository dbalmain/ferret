module Ferret::Search
  # Subclass of FilteredTermEnum for enumerating all terms that match the
  # specified wildcard filter term.
  # 
  # Term enumerations are always ordered by Term.compareTo().  Each term in
  # the enumeration is greater than all that precede it.
  # 
  class WildcardTermEnum < FilteredTermEnum 
    attr_reader :end_enum

    WILDCARD_STRING = '*'
    WILDCARD_CHAR = '?'

    # Creates a new +WildcardTermEnum+.  Passing in a
    # org.apache.lucene.index.Term Term that does not contain a
    # +WILDCARD_CHAR+ will cause an exception to be raisen.
    # 
    # After calling the constructor the enumeration is already pointing to the first 
    # valid term if such a term exists.
    def initialize(reader, term)
      super()
      @end_enum = false
      @search_term = term
      @field = @search_term.field
      text = @search_term.text
      len = text.length

      sidx = text.index(WILDCARD_STRING)||len
      cidx = text.index(WILDCARD_CHAR)||len
      idx = [sidx, cidx].min

      @pre = @search_term.text[0,idx]
      @pre_len = idx
      @pattern = /^#{Regexp.escape(text[idx..-1]).gsub(/\\([?*])/){".#{$1}"}}$/
      self.enum = reader.terms_from(Term.new(@search_term.field, @pre))
    end

    def term_compare(term) 
      if (@field == term.field) 
        search_text = term.text
        if (search_text[0, @pre_len] == @pre) 
          return (search_text[@pre_len..-1] =~ @pattern)
        end
      end
      @end_enum = true
      return false
    end

    def difference() 
      return 1.0
    end

    def close()
      super()
      @pattern = nil
      @field = nil
    end
  end
end
