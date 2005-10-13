module Ferret::Search::Spans
  # Matches spans near the beginning of a field. 
  class SpanFirstQuery < SpanQuery 
    # Construct a SpanFirstQuery matching spans in +match+ whose finish
    # position is less than or equal to +finish+. 
    def initialize(match, finish) 
      super()
      @match = match
      @finish = finish
    end

    # Return the SpanQuery whose matches are filtered. 
    def match() @match end

    # Return the maximum finish position permitted in a match. 
    def finish() @finish end

    def field() @match.field() end

    def terms() @match.terms() end

    def to_s(field = nil) 
      return "span_first(#{@match.to_s(field)}, #{finish})"
    end

    def spans(reader)
      SpanFirstEnum.new(self, reader) 
    end

    class SpanFirstEnum < SpansEnum
      def initialize(query, reader)
        super()
        @query = query
        @spans = @query.match.spans(reader)
      end

      def next?()
        while (@spans.next?()) # scan to next match
          return true if (finish() <= @query.finish)
        end
        return false
      end

      def skip_to(target)
        if not @spans.skip_to(target)
          return false
        end

        if (@spans.finish <= @query.finish) # there is a match
          return true
        end

        return next?()               # scan to next match
      end

      def doc() @spans.doc() end
      def start() @spans.start() end
      def finish() @spans.finish() end

      def to_s() "spans(#{@query})" end
    end


    def rewrite(reader)
      clone = nil
      rewritten = @match.rewrite(reader)
      if (rewritten != @match) 
        clone = self.clone()
        clone.match = rewritten
      end

      if (clone != nil) 
        return clone                        # some clauses rewrote
      else 
        return self                         # no clauses rewrote
      end
    end
  end
end
