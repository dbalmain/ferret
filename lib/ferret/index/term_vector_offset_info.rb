module Ferret::Index
  class TermVectorOffsetInfo 
    attr_accessor :start, :end

    def initialize(start, endd) 
      @end = endd
      @start = start
    end

    def eql?(o) 
      return false if !o.instance_of?(TermVectorOffsetInfo)
      @end == o.end and @start == o.start
    end
    alias :== :eql?

    def hash() 
      29 * @start + @end
    end
  end
end
