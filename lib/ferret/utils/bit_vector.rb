module Ferret::Utils
  # Optimized implementation of a vector of bits.
  #
  #  * a count() method, which efficiently computes the number of one bits
  #  * optimized read from and write to disk
  #  * inlinable get() method
  class BitVector 
    attr_reader :size
    attr_accessor :bits

    def initialize
      @bits = 0
      @count = -1
    end

    # Sets the value of _bit_ to one. 
    def set(bit) 
      @bits |= 1 << bit
      @count = -1
    end

    # Sets the value of _bit_ to zero. 
    def clear(bit) 
      @bits &= ~(1 << bit)
      @count = -1
    end

    # Returns _true_ if _bit_ is one and
    # _false_ if it is zero. 
    def get(bit) 
      return (@bits & (1 << bit)) != 0
    end
    alias :[] :get

    # Returns the total number of one bits in this vector.  This is
    # efficiently computed and cached, so that, if the vector is not
    # changed, no recomputation is done for repeated calls. 
    def count() 
      # if the vector has been modified
      if (@count == -1) 
        c = 0
        tmp = @bits
        while tmp > 0
          c += BYTE_COUNTS[tmp & 0xFF] # sum bits per byte
          tmp >>= 8
        end
        @count = c
      end
      return @count
    end

    BYTE_COUNTS = [ # table of bits/byte
      0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
      1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
      1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
    ] 


    # Writes this vector to the file _name_ in Directory _d_, in a format
    # that can be read by the constructor
    def write(d, name)
      output = d.create_output(name)
      begin 
        output.write_string(self.class.bignum_to_string(@bits))
      ensure 
        output.close()
      end
    end

    # Constructs a bit vector from the file _name_ in Directory _d_, as
    # written by the @link #writeendmethod.
    def BitVector.read(d, name)
      bv = BitVector.new
      input = d.open_input(name)
      begin 
        bv.bits = string_to_bignum(input.read_string())
      ensure 
        input.close()
      end
      return bv
    end

    def to_s
      i = @bits
      while i > 0
        print(i&1)
        i >>= 1
      end
      puts ""
    end

    # converts a BigNum into a string
    def BitVector.bignum_to_string(num)
      str = []
      while (num > 0)
        str << (num & 0xff)
        num >>= 8
      end
      return str.pack("C*")
    end

    # converts a string into a bignum
    def BitVector.string_to_bignum(str)
      str = str.unpack("C*") 
      num = 0
      str.reverse.each {|c| num = ((num << 8) | c) }
      return num
    end
  end
end
