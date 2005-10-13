module Ferret::Search::Spans
  # Matches spans containing a term. 
  class SpanTermQuery < SpanQuery 
    # Construct a SpanTermQuery matching the named term's spans. 
    def initialize(term)
      super()
      @term = term
    end

    # Return the term whose spans are matched. 
    def term() @term end

    def field() @term.field() end

    def terms() [@term] end

    def to_s(field = nil) 
      if @term.field == field
        return @term.text
      else
        return @term.to_s
      end
    end

    # Returns true iff +o+ is equal to this. 
    def eql?(o) 
      return (o.instance_of?(SpanTermQuery) and boost() == o.boost and @term == o.term)
    end
    alias :== :eql?

    # Returns a hash code value for this object.
    def hash()
      return boost().hash ^ @term.hash
    end

    def spans(reader)
      return SpanTermEnum.new(self, reader)
    end

    class SpanTermEnum < SpansEnum
      def initialize(query, reader) 
        @query = query
        @positions = reader.term_positions_for(@query.term)
        @position = -1
        @doc = -1
        @count = 0
        @freq = 0
      end

      def next?
        if (@count == @freq) 
          if not @positions.next?
            @doc = Scorer::MAX_DOCS
            return false
          end
          @doc = @positions.doc()
          @freq = @positions.freq()
          @count = 0
        end
        @position = @positions.next_position()
        @count += 1
        return true
      end

      def skip_to(target)
        # are we already at the correct position?
        if (@doc >= target) 
          return true
        end

        if not @positions.skip_to(target)
          @doc = Scorer::MAX_DOCS
          return false
        end

        @doc = @positions.doc()
        @freq = @positions.freq()
        @count = 0

        @position = @positions.next_position()
        @count += 1

        return true
      end

      def doc() @doc end
      def start() @position end
      def finish() @position + 1 end

      def to_s() 
        buffer = "spans(#{@query})@"
        if @doc < 0
          buffer << "START"
        else
          buffer << (@doc==Scorer::MAX_DOCS ? "END" : "#{@doc}-#{@position}")
        end
        return buffer
      end
    end

  end
end
