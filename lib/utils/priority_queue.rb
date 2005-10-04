module Ferret::Utils
  # A PriorityQueue maintains a partial ordering of its objects such that
  # the least object can always be found in constant time. push()'s and
  # pop()'s require log(size) time. The objects in this priority queue must
  # be Comparable
  class PriorityQueue 
    attr_reader :size

    def less_than(a, b)
      a < b
    end

    # Subclass constructors must call this. 
    def initialize(max_size)
      @size = 0
      @heap = Array.new(max_size + 1)
      @max_size = max_size
    end

    # Adds an Object to a PriorityQueue in log(size) time.
    #
    # If one tries to add more objects than max_size from initialize a
    # RuntimeException (ArrayIndexOutOfBound) is thrown.
    def push(object) 
      @size += 1
      @heap[@size] = object
      up_heap()
    end
    alias :<< :push

    # Adds object to the PriorityQueue in log(size) time if either the
    # PriorityQueue is not full, or not less_than(object, top()).
    #
    # object:: the object to be inserted
    # return true if object is added, false otherwise.
    def insert(object)
      if(@size < @max_size)
        push(object)
        return true
      elsif (@size > 0 and less_than(top, object))
        @heap[1] = object
        down_heap()
        return true
      else
        return false
      end
     end

    # Returns the least object of the PriorityQueue in constant time. 
    def top
      return @heap[1]
    end

    # Removes and returns the least object of the PriorityQueue in log(size)
    # time. 
    def pop() 
      if (@size > 0) 
        result = @heap[1]        # save first value
        @heap[1] = @heap[@size]  # move last to first
        @heap[@size] = nil;      # permit GC of objects
        @size -= 1
        down_heap()              # adjust heap
        return result
      else
        return nil
      end
    end

    # Removes all entries from the PriorityQueue. 
    def clear() 
      (1..@size).each do |i|
        @heap[i] = nil
      end
      @size = 0
    end

    # resets the queue after the top has been changed
    def adjust_top()
      down_heap()
    end

    private

      def up_heap() 
        i = @size
        node = @heap[i]             # save bottom node
        j = i >> 1
        while (j > 0 and less_than(node, @heap[j])) 
          @heap[i] = @heap[j];      # shift parents down
          i = j
          j = j >> 1
        end
        @heap[i] = node;            # install saved node
      end

      def down_heap() 
        i = 1
        node = @heap[i]             # save top node
        j = i << 1                  # find smaller child
        k = j + 1
        if k <= @size and less_than(@heap[k], @heap[j])
          j = k
        end
        while (j <= @size and less_than(@heap[j], node)) 
          @heap[i] = @heap[j]       # shift up child
          i = j
          j = i << 1
          k = j + 1
          if k <= @size and less_than(@heap[k], @heap[j]) 
            j = k
          end
        end
        @heap[i] = node;            # install saved node
      end
  end
end
