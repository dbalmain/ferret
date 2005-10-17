module Ferret::Search::Spans
  # Matches spans which are near one another.  One can specify _slop_, the
  # maximum number of intervening unmatched positions, as well as whether
  # matches are required to be in-order. 
  class SpanNearQuery < SpanQuery 

    # Construct a SpanNearQuery.  Matches spans matching a span from each
    # clause, with up to +slop+ total unmatched positions between them. When
    # +in_order+ is true, the spans from each clause must be ordered as in
    # +clauses+. 
    def initialize(clauses, slop, in_order) 
      super()
      # copy clauses array into an ArrayList
      @clauses = Array.new(clauses.length)
      @field = nil
      clauses.each_index do |i|
        clause = clauses[i]
        if i == 0 # check field
          @field = clause.field()
        elsif clause.field() != @field
          raise ArgumentError, "Clauses must have same field."
        end
        @clauses[i] = clause
      end

      @slop = slop
      @in_order = in_order
    end

    # Return the clauses whose spans are matched. 
    def clauses() @clauses end

    # Return the maximum number of intervening unmatched positions permitted.
    def slop() @slop end

    # Return true if matches are required to be in-order.
    def in_order?() @in_order end

    attr_reader :field

    def terms() 
      terms = []
      @clauses.each do |clause|
        terms += clause.terms
      end
      return terms
    end

    def to_s(field = nil) 
      buffer = "span_near(["
      buffer << @clauses.map {|c| c.to_s(field)}.join(", ")
      buffer << "], #{@stop}, #{@in_order})"
      return buffer
    end

    def spans(reader)
      if (@clauses.size() == 0)                      # optimize 0-clause case
        return SpanOrQuery.new(@clauses).spans(reader)
      end

      if (@clauses.size() == 1)                      # optimize 1-clause case
        return @clauses[0].spans(reader)
      end

      return NearSpansEnum.new(self, reader)
    end

    def rewrite(reader)
      clone = nil
      @clauses.each_index do |i|
        clause = @clauses[i]
        query = clause.rewrite(reader)
        if (query != clause) # clause rewrote: must clone
          if (clone == nil)
            clone = self.clone()
          end
          clone.clauses[i] = query
        end
      end
      if (clone != nil) 
        return clone                        # some clauses rewrote
      else 
        return self                         # no clauses rewrote
      end
    end

    # Returns true iff +o+ is equal to this. 
    def eql?(o) 
      return false if (o == nil or self.class() != o.class())

      return false if (@in_order != o.in_order?)
      return false if (@slop != o.slop)
      return false if (@clauses != o.clauses)
      return false if (@field != o.field)

      return true
    end
    alias :== :eql?

    def hash() 
      result = @clauses.hash()
      result += @slop * 29
      result +=  (@in_order ? 1 : 0)
      result ^= @field.hash()
      return result
    end
  end
end
