module Ferret
  module Store
    # Ferret's IO Input methods are defined here. The methods read_byte and
    # read_bytes need to be defined before this class is of any use. 
    class IndexInput

      # Reads and returns a single byte.
      def read_byte()
        raise NotImplementedError
      end

      # Reads a specified number of bytes into an array at the specified offset.
      # buf:: the array to read bytes into
      # offset:: the offset in the array to start storing bytes
      # len:: the number of bytes to read
      def read_bytes(buf, offset, len)
        raise NotImplementedError
      end
       

      # Reads four bytes and returns an int. read_uint should be used for
      # unsigned integers for performance reasons.
      def read_int
        # This may be slow. I'm not sure if this is the best way to get
        # integers from files but this is the only way I could find to get
        # signed integers.
        #i = read_byte
        #return (((i&0x80)==0 ? 0 : -1) << 32) |
               #(i << 24) |
               #((read_byte) << 16) |
               #((read_byte) << 8) |
               #(read_byte)
        i1 = read_byte
        i2 = read_byte
        i3 = read_byte
        i4 = read_byte
        res =  (((i1&0x80) == 0 ? 0xffffffff : -0x100000000)) |
               ((i1 << 24) | (i2 << 16) | (i3 << 8) | (i4))
        puts "#{(((i1&0x80) == 0 ? 0 : -1) << 32).to_s(16)}:#{(i1 << 24).to_s(16)}:#{(i2 << 16).to_s(16)}:#{(i3 << 8).to_s(16)}:#{i4.to_s(16)} = #{res.to_s(16)}"
        return res
      end

      # Reads eight bytes and returns a long.
      def read_long
        return (read_int << 32 | read_int & 0xFFFFFFFF)
      end

      # Reads four bytes and returns a positive integer
      def read_uint
        return ((read_byte) << 24) | ((read_byte) << 16) |
               ((read_byte) <<  8) |  (read_byte)
      end
      
      # Reads eight bytes and returns a positive integer.
      def read_ulong
        return (read_uint << 32) | (read_uint & 0xFFFFFFFF)
      end

      # Reads an int stored in variable-length format.  Reads between one and
      # five bytes.  Smaller values take fewer bytes.  Negative numbers are not
      # supported.
      def read_vint
        b = read_byte
        i = b & 0x7F # 0x7F = 0b01111111
        shift = 7
        
        while b & 0x80 != 0 # 0x80 = 0b10000000
          b = read_byte
          i |= (b & 0x7F) << shift
          shift += 7
        end
        
        return i
      end
      alias :read_vlong :read_vint

      # Reads a string. A string is stored as a single vint which describes
      # the length of the string, followed by the actually string itself.
      def read_string
        length = read_vint
        
        chars = Array.new(length, ' ')
        read_chars(chars, 0, length)
        
        chars.to_s
      end

      # Reads UTF-8 encoded characters into an array.
      # buf:: the array to read characters into
      # start:: the offset in the array to start storing characters
      # length:: the number of characters to read
      #
      # TODO: Test on some actual UTF-8 documents.
      def read_chars(buf, start, length)
        if buf.length < (start + length)
          # make room for the characters to read
          buf << " " * (start + length - buf.length)
        end
        last = start + length
        (start...last).each do |i|
          buf[i] = read_byte.chr
        end
#        last = start + length
#        
#        (start...last).each do |i|
#          b = read_byte
#          if (b & 0x80) == 0
#            buf[i] = (b & 0x7F).chr # don't need to worry about UTF-8 here
#          else
#            if (b & 0xE0) != 0xE0
#              tmp_int = (((b & 0x1F) << 6) | (read_byte & 0x3F))
#              buf[i] = [tmp_int].pack("C") # pack into a UTF-8 string
#            else
#              buf[i] = [
#                         ((b & 0x0F) << 12) |
#                         ((read_byte & 0x3F) << 6) |
#                         (read_byte & 0x3F)
#                       ].pack("U") # pack into a UTF-8 string
#            end
#          end
#        end
      end
      
      # Closes the stream to futher operations. 
      def close
        raise NotImplementedError
      end

      # Returns the current position in this file, where the next read will
      # occur.
      def pos
        raise NotImplementedError
      end

      # Sets current position in this file, where the next read will occur.
      def seek(pos)
        raise NotImplementedError
      end

      # The number of bytes in the file. 
      def length
        raise NotImplementedError
      end

      # Returns a clone of this stream.
      # 
      # Clones of a stream access the same data, and are positioned at the same
      # point as the stream they were cloned from.
      # 
      # Expert:: Subclasses must ensure that clones may be positioned at
      # different points in the input from each other and from the stream they
      # were cloned from.
      #   def clone
      #     raise NotImplementedError
      #   end

    end

    # Ferret's IO Output methods are defined here. The methods write_byte and
    # write_bytes need to be defined before this class is of any use. 
    class IndexOutput 

      # Writes a single byte.
      def write_byte(b)
        raise NotImplementedError
      end

      # Writes an array of bytes.
      # buf:: the bytes to write
      # len:: the number of bytes to write
      def write_bytes(buf, len)
        raise NotImplementedError
      end

      # Writes an int as four bytes.
      def write_int(i)
        write_byte((i >> 24) & 0xFF)
        write_byte((i >> 16) & 0xFF)
        write_byte((i >>  8) & 0xFF)
        write_byte(i & 0xFF)
      end
      alias :write_uint :write_int

      # Writes an int in a variable-length format.  Writes between one and
      # five bytes.  Smaller values take fewer bytes.  Negative numbers are not
      # supported.
      def write_vint(i)
        while i > 127
          write_byte((i & 0x7f) | 0x80)
          i >>= 7
        end
        write_byte(i)
      end
      alias :write_vlong :write_vint

      # Writes a long as eight bytes.
      def write_long(i)
        write_int(i >> 32)
        write_int(i)
      end
      alias :write_ulong :write_long

      # Writes a string.
      def write_string(s)
        length = s.length()
        write_vint(length)
        write_chars(s, 0, length)
      end

      # Writes a sequence of UTF-8 encoded characters from a string.
      # buf:: the source of the characters
      # start:: the first character in the sequence
      # length:: the number of characters in the sequence
      def write_chars(buf, start, length)
        last = start + length
        (start ... last).each do |i|
          write_byte(buf[i])
#          code = buf[i]
#          if code >= 0x01 and code <= 0x7F
#            write_byte(code)
#          else
#            # We need to write unicode characters. ToDo: test that this works.
#            if code > 0x80 and code <= 0x7FF or code == 0
#              write_byte(0xC0 | code >> 6)
#              write_byte(0x80 | code & 0x3F)
#            else
#              write_byte(0xE0 | (code >> 12))
#              write_byte(0x80 | ((code >> 6) & 0x3F))
#              write_byte(0x80 | (code & 0x3F))
#            end
#          end
        end
      end

      # Forces any buffered output to be written. 
      def flush
        raise NotImplementedError
      end

      # Closes this stream to further operations. 
      def close
        raise NotImplementedError
      end

      # Returns the current position in this file, where the next write will
      # occur.
      def pos
        raise NotImplementedError
      end

      # Sets current position in this file, where the next write will occur.
      def seek(pos)
        raise NotImplementedError
      end

      # The number of bytes in the file. 
      def length
        raise NotImplementedError
      end
    end
  end
end
