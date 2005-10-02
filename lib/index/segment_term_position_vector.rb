module Ferret::Index
  class SegmentTermPositionVector < SegmentTermVector
    attr_reader :positions, :offsets
    
    def initialize(field, terms, term_freqs, positions, offsets) 
      super(field, terms, term_freqs)
      @offsets = offsets
      @positions = positions
    end
  end
end
