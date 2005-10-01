module Ferret
  module Utils
    # A PriorityQueue maintains a partial ordering of its objects such that
    # the least object can always be found in constant time. put()'s and
    # pop()'s require log(size) time. The objects in this priority queue must
    # be Comparable
    class PriorityQueue 
      attr_reader :size
      def less_than(a, b)
        a < b
      end

      # Subclass constructors must call this. 
      def initialize(max_size)
        @heap = []
      end

      def size
        @heap.size
      end

      # Adds an Object to a PriorityQueue in log(size) time.
      #
      # If one tries to add more objects than max_size from initialize a
      # RuntimeException (ArrayIndexOutOfBound) is thrown.
      def push(object) 
        @heap << object
        #up_heap()
      end
      alias :<< :push
      alias :insert :push

      # Adds object to the PriorityQueue in log(size) time if either the
      # PriorityQueue is not full, or not less_than(object, top()).
      #
      # object:: the object to be inserted
      # return true if object is added, false otherwise.

#      def insert(object)
#        if(@size < max_size)
#          put(object)
#          return true
#        elsif (@size > 0 and less_than(top, object))
#          @heap[1] = object
#          down_heap()
#          return true
#        else
#          return false
#        end
#       end

      # Returns the least object of the PriorityQueue in constant time. 
      def top
        @heap.sort! {|a,b| less_than(b,a)}
        return @heap.last
      end

      # Removes and returns the least object of the PriorityQueue in log(size)
      # time. 
      def pop() 
        @heap.sort! {|a,b| less_than(b,a)}
        res = @heap.pop
      end

      # Removes all entries from the PriorityQueue. 
      def clear() 
        @heap.clear
      end

      # resets the queue after the top has been changed
    end
  end
end
