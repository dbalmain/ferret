module Ferret::Search
  class PhrasePositions 
    attr_reader :doc, :position
    attr_accessor :next

    def initialize(tp_enum, offset) 
      @tp_enum = tp_enum
      @offset = offset
      @count = @position = @doc = -1
      @next = nil
    end

    def next?()
      if not @tp_enum.next?
        @tp_enum.close()          # close stream
        @doc = Scorer::MAX_DOCS    # sentinel value
        return false
      end
      @doc = @tp_enum.doc
      @position = 0
      return true
    end

    def skip_to(target)
      if not @tp_enum.skip_to(target)
        @tp_enum.close()          # close stream
        @doc = Scorer::MAX_DOCS    # sentinel value
        return false
      end
      @doc = @tp_enum.doc
      @position = 0
      return true
    end


    def first_position()
      @count = @tp_enum.freq       # read first pos
      next_position()
    end

    def next_position()
      @count -= 1
      if @count >= 0          # read subsequent pos's
        @position = @tp_enum.next_position() - @offset
        return true
      else
        return false
      end
    end
  end
end
