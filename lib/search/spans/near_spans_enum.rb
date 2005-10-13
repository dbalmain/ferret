module Ferret::Search::Spans
  class NearSpansEnum < SpansEnum

    class CellQueue < PriorityQueue 
      def less_than(o1, o2) 
        if (o1.doc == o2.doc) 
          if (o1.start == o2.start) 
            if (o1.finish == o2.finish) 
              return o1.index > o2.index
            else 
              return o1.finish < o2.finish
            end
          else 
            return o1.start < o2.start
          end
        else 
          return o1.doc < o2.doc
        end
      end
    end


    # Wraps a SpansEnum, and can be used to form a linked list.
    class SpansCell < SpansEnum
      attr_accessor :next, :index

      def initialize(parent, spans, index) 
        @parent = parent
        @spans = spans
        @index = index
        @length = -1
      end

      def next?()
        if (@length != -1)                  # subtract old length
          @parent.total_length -= @length
        end

        more = @spans.next?                 # move to next

        if more 
          @length = finish() - start()      # compute new length
          @parent.total_length += @length   # add new length to total

          if (@parent.max.nil? or doc() > @parent.max.doc or     # maintain max
              (doc() == @parent.max.doc and finish() > @parent.max.finish))
            @parent.max = self
          end
        end

        return more
      end

      def skip_to(target)
        if (@length != -1)                  # subtract old length
          @parent.total_length -= @length
        end

        more = @spans.skip_to(target)       # skip

        if (more) 
          @length = finish() - start()      # compute new length
          @parent.total_length += @length   # add new length to total

          if (@parent.max == nil or doc() > @parent.max.doc() or   # maintain max
              (doc() == @parent.max.doc and finish() > @parent.max.finish))
            @parent.max = self
          end
        end

        return more
      end

      def doc() return @spans.doc() end
      def start() return @spans.start() end
      def finish() return @spans.finish() end

      def to_s() return "#{@spans}##{@index}" end
    end

    attr_accessor :total_length, :max

    def initialize(query, reader)
      @ordered = []         # spans in query order

      @first = nil          # linked list of spans
      @last = nil           # sorted by doc only

      @total_length = 0     # sum of current lengths

      @queue = nil          # sorted queue of spans
      @max = nil            # max element in queue

      @more = true          # true iff not done
      @first_time = true    # true before first next?

     
      @query = query
      @slop = query.slop
      @in_order = query.in_order?

      clauses = query.clauses # initialize spans & list
      @queue = CellQueue.new(clauses.length)
      clauses.length.times do |i|
        # construct clause spans
        cell = SpansCell.new(self, clauses[i].spans(reader), i)
        @ordered << cell    # add to ordered
      end
    end

    def next?()
      if (@first_time) 
        init_list(true)
        list_to_queue()                # initialize queue
        @first_time = false
      elsif (@more) 
        @more = min().next?            # trigger further scanning
        @queue.adjust_top() if (@more) # maintain queue 
      end

      while (@more) 
        queue_stale = false

        if (min().doc != @max.doc)     # maintain list
          queue_to_list()
          queue_stale = true
        end

        # skip to doc w/ all clauses

        while (@more and @first.doc < @last.doc) 
          @more = @first.skip_to(@last.doc) # skip first upto last
          first_to_last()              # and move it to the end
          queue_stale = true
        end

        return false if not @more

        # found doc w/ all clauses

        if (queue_stale) # maintain the queue
          list_to_queue()
          queue_stale = false
        end

        return true if at_match?
        
        # trigger further scanning
        if (@in_order and check_slop?()) 
          # There is a non ordered match within slop and an ordered match is needed. 
          @more = first_non_ordered_next_to_partial_list()
          if (@more) 
            partial_list_to_queue()
          end
        else 
          @more = min().next?()
          if (@more) 
            @queue.adjust_top()        # maintain queue
          end
        end
      end
      return false                     # no more matches
    end

    def each()
      cell = @first
      while (cell)
        yield cell
        cell=cell.next
      end
    end

    def skip_to(target)
      if (@first_time) # initialize
        init_list(false)
        each() do |cell|
          @more = cell.skip_to(target) # skip all
          break if not @more
        end

        if (@more) 
          list_to_queue()
        end
        @first_time = false

      else # normal case
        while (@more and min().doc < target) # skip as needed
          @more = min().skip_to(target)
          @queue.adjust_top() if (@more)
        end
      end

      if (@more) 
        return true if (at_match?())              # at a match?
        return next?                              # no, scan
      end

      return false
    end

    def min() @queue.top() end

    def doc() min().doc() end
    def start() min().start() end
    def finish() @max.finish() end


    def to_s() 
      buffer = "spans(#{@query})@"
      if @first_time
        buffer << "START"
      else
        buffer << (@queue.size>0 ? ("#{doc}:#{start()}-#{finish}") : "END")
      end
      return buffer
    end

    def init_list(nxt)
      @ordered.each do |cell|
        @more = cell.next? if nxt
        if @more
          add_to_list(cell) # add to list
        else
          break
        end
      end
    end

    def add_to_list(cell) 
      if (@last != nil) # add next to end of list
        @last.next = cell
      else
        @first = cell
      end
      @last = cell
      cell.next = nil
    end

    def first_to_last() 
      @last.next = @first # move first to end of list
      @last = @first
      @first = @first.next
      @last.next = nil
    end

    def queue_to_list() 
      @last = @first = nil
      while (@queue.top() != nil) 
        add_to_list(@queue.pop())
      end
    end
    
    def first_non_ordered_next_to_partial_list()
      # Creates a partial list consisting of first non ordered and earlier.
      # Returns first non ordered .next?.
      @last = @first = nil
      ordered_index = 0
      while (@queue.top() != nil) 
        cell = @queue.pop()
        add_to_list(cell)
        if (cell.index == ordered_index) 
          ordered_index += 1
        else 
          return cell.next?()
          # FIXME: continue here, rename to eg. checkOrderedMatch():
          # when check_slop?() and not ordered, repeat cell.next?().
          # when check_slop?() and ordered, add to list and repeat queue.pop()
          # without check_slop?(): no match, rebuild the queue from the partial list.
          # When queue is empty and check_slop?() and ordered there is a match.
        end
      end
      raise RuntimeException, "Unexpected: ordered"
    end

    def list_to_queue() 
      @queue.clear() # rebuild queue
      partial_list_to_queue()
    end

    def partial_list_to_queue() 
      each() { |cell| @queue.push(cell) } # add to queue from list
    end

    def at_match?() 
      return ((min().doc() == @max.doc()) and check_slop?() and
              (not @in_order or match_is_ordered?()))
    end
    
    def check_slop?() 
      match_length = @max.finish() - min.start()
      return ((match_length - @total_length) <= @slop)
    end

    def match_is_ordered?() 
      last_start = -1
      @ordered.each do |cell|
        start = cell.start
        return false if start <= last_start
        last_start = start
      end
      return true
    end
  end
end
