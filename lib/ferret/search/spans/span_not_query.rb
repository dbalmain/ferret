module Ferret::Search::Spans
  # Removes matches which overlap with another SpanQuery. 
  class SpanNotQuery < SpanQuery 
    # Construct a SpanNotQuery matching spans from +incl+ which
    # have no overlap with spans from +excl+.
    def initialize(incl, excl) 
      super()
      @incl = incl
      @excl = excl

      if incl.field != excl.field
        raise ArgumentError, "Clauses must have same field."
      end
    end

    # Return the SpanQuery whose matches are filtered. 
    def incl() @incl end

    # Return the SpanQuery whose matches must not overlap those returned. 
    def excl() @excl end

    def field() @incl.field() end

    def terms() @incl.terms() end

    def to_s(field = nil) 
      return "span_not(#{incl.to_s(field)}, #{excl.to_s(field)})"
    end

    def spans(reader)
      return SpanNotEnum.new(self, reader)
    end
    
    class SpanNotEnum < SpansEnum
      def initialize(query, reader)
        @query = query
        @incl_spans = @query.incl.spans(reader)
        @more_incl = true
        @excl_spans = @query.excl.spans(reader)
        @more_excl = @excl_spans.next? # excl_spans needs to be preset
      end

      def next?()
        if (@more_incl)                          # move to next incl
          @more_incl = @incl_spans.next?()
        end

        while (@more_incl and @more_excl) 
          if (@incl_spans.doc > @excl_spans.doc) # skip excl
            @more_excl = @excl_spans.skip_to(@incl_spans.doc)
          end

          while (@more_excl and                  # while excl is before
                 @incl_spans.doc == @excl_spans.doc and
                 @excl_spans.finish <= @incl_spans.start) 
            @more_excl = @excl_spans.next?       # increment excl
          end

          if (not @more_excl or                  # if no intersection
              @incl_spans.doc != @excl_spans.doc or
              @incl_spans.finish <= @excl_spans.start)
            break                                # we found a match
          end

          @more_incl = @incl_spans.next?         # intersected: keep scanning
        end
        return @more_incl
      end

      def skip_to(target)
        if @more_incl                            # skip incl
          @more_incl = @incl_spans.skip_to(target)
        end

        if not @more_incl
          return false
        end

        if (@more_excl and @incl_spans.doc > @excl_spans.doc) # skip excl
          @more_excl = @excl_spans.skip_to(@incl_spans.doc)
        end

        while (@more_excl and                    # while excl is before
               @incl_spans.doc == @excl_spans.doc and
               @excl_spans.finish <= @incl_spans.start) 
          @more_excl = @excl_spans.next?         # increment excl
        end

        if (not @more_excl or                    # if no intersection
              @incl_spans.doc != @excl_spans.doc or
              @incl_spans.finish <= @excl_spans.start)
          return true                            # we found a match
        end

        return next?()                           # scan to next match
      end

      def doc() @incl_spans.doc end
      def start() @incl_spans.start end
      def finish() @incl_spans.finish end

      def to_s() 
        return "spans(#{@query})"
      end
    end

    def rewrite(reader)
      clone = nil

      rewritten_incl = @incl.rewrite(reader)
      if (rewritten_incl != @incl) 
        clone = self.clone()
        clone.incl = rewritten_incl
      end

      rewritten_excl = @excl.rewrite(reader)
      if (rewritten_excl != @excl) 
        clone = self.clone() if (clone == nil)
        clone.excl = rewritten_excl
      end

      if (clone != nil) 
        return clone                        # some clauses rewrote
      else 
        return self                         # no clauses rewrote
      end
    end

  end
end
