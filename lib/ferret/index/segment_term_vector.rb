module Ferret::Index
  # Provides access to stored term vector of 
  # a document field.
  class SegmentTermVector
    # Array of term frequencies. Locations of the array correspond one to one
    # to the terms in the array obtained from _terms_
    # method. Each location in the array contains the number of times this
    # term occurs in the document or the document field.
    attr_reader :term_frequencies, :positions, :offsets

    attr_reader :field, :terms

    def initialize(field, terms, term_freqs, positions=nil, offsets=nil) 
      @field = field
      @terms = terms
      @term_frequencies = term_freqs
      @positions = positions
      @offsets = offsets
    end

    def to_s() 
      sb = @field.to_s + ": "
      if @terms
        terms.each_with_index do |term, i|
          sb << ', ' if i > 0
          sb << term + '/' + @term_frequencies[i].to_s
        end
      end
      sb << 'end'
      
      return sb
    end

    # Returns the number of unique terms in the field
    def size() 
      return @terms == nil ? 0 : @terms.size
    end

    # Return an index in the term numbers array returned from _get_terms_ at
    # which the term with the specified _term_ appears. If this term does
    # not appear in the array, return -1.
    def index_of(term) 
      return @terms ? @terms.index(term) : nil
    end

    # Just like _index_of_ but searches for a number of terms at the same
    # time. Returns an array that has the same size as the number of terms
    # searched for, each slot containing the result of searching for that
    # term number.
    #
    # terms:: array containing terms to look for
    # start:: index in the array where the list of terms starts
    # len:: the number of terms in the list
    def indexes_of(terms, start, len) 
      return terms[start, len].map { |term| index_of(term) }
    end
  end
end
