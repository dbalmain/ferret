module Ferret::Utils
  module StringHelper 
  # Methods for manipulating strings.

    class StringReader
      attr_reader :length

      def initialize(str)
        @str = str
        @pointer = 0
        @length = @str.length
      end

      def read(len = nil)
        return @str if len.nil?

        return nil if @pointer > @length

        res = @str[@pointer, len]
        @pointer += len
        return res
      end

      def reset() @pointer = 0 end

      def close() str = nil end
    end

    # Compares two strings, character by character, and returns the
    # first position where the two strings differ from one another.
    # eg.
    #   string_difference('dustbin', 'dusty') # => 4
    #   string_difference('dustbin', 'evening') # => 0
    #   string_difference('eve', 'evening') # => 3
    # 
    # s1:: The first string to compare
    # s2:: The second string to compare
    # returns:: The first position where the two strings differ.
    def string_difference(s1, s2) 
      len = [s1.length, s2.length].min
      len.times do |i|
        return i if (s1[i] != s2[i]) 
      end
      return len
    end
  end
end
