module Ferret
  module Index
    class TermVectorOffsetInfo 
      attr_accessor :start_offset, :end_offset

      def initialize(start_offset, end_offset) 
        @end_offset = end_offset
        @start_offset = start_offset
      end

      def eql?(o) 
        return false if !o.instance_of?(TermVectorOffsetInfo)
        @end_offset == o.end_offset and @start_offset == o.start_offset
      end
      alias :== :eql?

      def hash() 
        29 * @start_offset + @end_offset
      end
    end
  end
end
