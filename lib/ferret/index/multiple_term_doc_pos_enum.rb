module Ferret::Index
  # Describe class +MultipleTermPositions+ here.
  # 
  # @author Anders Nielsen
  class MultipleTermDocPosEnum < TermDocEnum 

    attr_accessor :doc, :freq
    class TermPositionsQueue < Ferret::Utils::PriorityQueue 
      def initialize(term_positions)
        super(term_positions.size)

        term_positions.each do |tp|
          push(tp) if tp.next?
        end
      end

      def less_than(tp1, tp2) 
        return tp1.doc < tp2.doc
      end
    end

    # Creates a new +MultipleTermPositions+ instance.
    # 
    # @exception IOException
    def initialize(reader, terms)
      term_positions = []

      terms.each do |term|
        term_positions << reader.term_positions_for(term)
      end

      @tps_queue = TermPositionsQueue.new(term_positions)
      @pos_list = []
    end

    def next?
      return false if (@tps_queue.size == 0)

      @pos_list.clear()
      @doc = @tps_queue.top.doc

      tps = nil
      begin 
        tps = @tps_queue.top()

        tps.freq.times do |i|
          @pos_list << tps.next_position()
        end

        if tps.next?
          @tps_queue.adjust_top()
        else 
          @tps_queue.pop()
          tps.close()
        end
      end while (@tps_queue.size > 0 and @tps_queue.top.doc == @doc)

      @pos_list.sort!()
      @freq = @pos_list.size

      return true
    end

    def next_position() 
      return @pos_list.shift()
    end

    def skip_to(target)
      while (@tps_queue.top != nil and target > @tps_queue.top.doc) 
        tps = @tps_queue.pop()
        if (tps.skip_to(target))
          @tps_queue.push(tps)
        else
          tps.close()
        end
      end
      return next?
    end

    def close()
      while (tps = @tps_queue.pop())
        tps.close()
      end
    end

    # Not implemented.
    # raises:: NotImplementedError
    def seek(term)
      raise NotImplementedError
    end

    # Not implemented.
    # raises:: NotImplementedError
    def read(docs, freqs)
      raise NotImplementedError
    end
  end
end
