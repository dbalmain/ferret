module Ferret::Search::Spans
  # Matches the union of its clauses.
  class SpanOrQuery < SpanQuery 

    # Construct a SpanOrQuery merging the provided clauses. 
    def initialize(clauses) 
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
    end

    # Return the clauses whose spans are matched. 
    def clauses() @clauses end

    attr_reader :field

    def terms() 
      terms = []
      @clauses.each do |clause|
        terms += clause.terms
      end
      return terms
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

    def to_s(field = nil) 
      buffer = "spanOr(["
      buffer << @clauses.map {|c| c.to_s(field()) }.join(", ")
      buffer << "])"
      return buffer
    end

    def eql?(o) 
      return false if (o == nil or self.class() != o.class())

      return false if (@clauses != o.clauses)
      return false if (@field != o.field)

      return true
    end
    alias :== :eql?

    def hash() 
      return @clauses.hash ^ @field.hash
    end

    class SpanQueue < Ferret::Utils::PriorityQueue 
      def less_than(o1, o2) 
        if (o1.doc == o2.doc) 
          if (o1.start == o2.start) 
            return o1.finish < o2.finish
          else 
            return o1.start < o2.start
          end
        else 
          return o1.doc < o2.doc
        end
      end
    end

    def spans(reader)
      if (@clauses.size == 1)                      # optimize 1-clause case
        return @clauses[0].spans(reader)
      end

      return SpanOrEnum.new(self, reader) 
    end

    class SpanOrEnum < SpansEnum
      def initialize(query, reader)
        @query = query
        @queue = SpanQueue.new(query.clauses.size)
        @all = query.clauses.map {|c| c.spans(reader)}
        @first_time = true
      end

      def next?
        if (@first_time) # first time -- initialize
          @all.delete_if do |spans|
            if (spans.next?) # move to first entry
              @queue.push(spans) # build queue
              next false
            else 
              next true
            end
          end
          @first_time = false
          return @queue.size() != 0
        end

        if @queue.size == 0 # all done
          return false
        end

        if top().next? # move to next
          @queue.adjust_top()
          return true
        end

        @all.delete(@queue.pop()) # exhausted a clause

        return @queue.size() != 0
      end

      def top() return @queue.top() end

      def skip_to(target)
        if (@first_time) 
          @all.delete_if do |spans|
            if (spans.skip_to(target)) # skip each spans in all
              @queue.push(spans) # build queue
              next false
            else 
              next true
            end
          end
          @first_time = false
        else 
          while (@queue.size != 0 and top().doc < target) 
            if (top().skip_to(target)) 
              @queue.adjust_top()
            else 
              @all.delete(@queue.pop())
            end
          end
        end

        return @queue.size() != 0
      end

      def doc() top().doc() end
      def start() top().start() end
      def finish() top().finish() end

      def to_s() 
        buffer = "spans(#{@query})@"
        if @first_time
          buffer << "START"
        else
          buffer << (@queue.size>0 ? ("#{doc}:#{start()}-#{finish}") : "END")
        end
        return buffer
      end
    end

  end
end
