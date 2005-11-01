module Ferret::Store
  BUFFER_SIZE = 1024
  BUFFER = " " * BUFFER_SIZE

  # Base implementation class for a buffered IndexOutput. 
  class BufferedIndexOutput < IndexOutput 

    def initialize
      @buffer = BUFFER.clone
      @buffer_start = 0          # position in file of buffer
      @buffer_position = 0       # position in buffer
    end

    # Writes a single byte.
    def write_byte(b)
    
      # The following code offers a 5% speed improvement over the line
      # below. It relies on the fact that ruby will throw an error if we try
      # and modify a character that is out of range for the string.
      #begin
      #  @buffer[@buffer_position] = b
      #  @buffer_position += 1
      #rescue IndexError
      #  flush
      #  @buffer[@buffer_position] = b
      #  @buffer_position += 1
      #end

      flush if @buffer_position >= BUFFER_SIZE
      @buffer[@buffer_position] = b
      @buffer_position += 1
    end

    # Writes an array of bytes.
    # buf:: the bytes to write
    # length:: the number of bytes to write
    def write_bytes(buf, length)
      length.times do |i|
        write_byte(buf[i])
      end
    end

    # Forces any buffered output to be written. 
    def flush()
      flush_buffer(@buffer, @buffer_position)
      @buffer_start += @buffer_position
      @buffer_position = 0
    end

    # Closes this stream to further operations. 
    def close()
      flush()
    end

    # Get the current position in the file, where the next write will occur.
    def pos() 
      return @buffer_start + @buffer_position
    end

    # Set the current position in the file, where the next write will occur.
    def seek(pos)
      flush()
      @buffer_start = pos
    end

    # The number of bytes in the file. 
    def length
      raise NotImplementedError
    end
    
    private

      # Expert: implements buffer write.  Writes the first len bytes from the
      # buffer to the output.
      # 
      # buf:: the bytes to write
      # len:: the number of bytes to write
      def flush_buffer(buf, len)
        raise NotImplementedError
      end
  end

  # Base implementation class for buffered IndexInput
  class BufferedIndexInput < IndexInput 
    def initialize
      @buffer = nil
      @buffer_start = 0
      @buffer_length = 0
      @buffer_position = 0
    end

    # Read and return a single byte from the file
    def read_byte
      refill if (@buffer_position >= @buffer_length)
      byte = @buffer[@buffer_position]
      @buffer_position += 1
      return byte
    end

    # Read +len+ bytes into +buffer+ starting at position +offset+ in +buffer+
    #
    # buffer:: The string buffer to read the characters into.
    # offset:: The position in +buffer+ to start writing to.
    # len:: the number of characters to read
    # returns:: the buffer
    def read_bytes(buffer, offset, len)
      if (len < BUFFER_SIZE) 
        offset.upto(offset+len-1) do |i| # read byte-by-byte
          buffer[i] = read_byte
        end
      else                               # read all-at-once
        start = pos()
        seek_internal(start)
        read_internal(buffer, offset, len)

        @buffer_start = start + len      # adjust stream variables
        @buffer_position = 0
        @buffer_length = 0               # trigger refill on read
      end
      return buffer
    end

    # Get the current position in the file, where the next read will occur.
    def pos()
      return @buffer_start + @buffer_position
    end

    # Set the current position in the file, where the next read will occur.
    def seek(pos)
      if (pos >= @buffer_start and pos < (@buffer_start + @buffer_length))
        @buffer_position = pos - @buffer_start  # seek within buffer
      else 
        @buffer_start = pos
        @buffer_position = 0
        @buffer_length = 0               # trigger refill() on read()
        seek_internal(pos)
      end
    end

    # Creates a clone of the BufferedIndexReader. Reading from a
    # BufferedIndexInput should not change the state (read position) in the
    # clone and vice-versa.
    def initialize_copy(o)
      super
      @buffer = o.buffer.clone if o.buffer
    end

    attr_reader :buffer
    protected :buffer

    private

      # Expert: implements buffer refill.  Reads bytes from the current position
      # in the input.
      # buf:: the array to read bytes into
      # offset:: the offset in the array to start storing bytes
      # len:: the number of bytes to read
      def read_internal(buf, offset, len)
        raise NotImplementedError
      end
          
      # Expert: implements seek.  Sets current position in this file, where the
      # next read_internal will occur.
      # pos:: the position to set to
      def seek_internal(pos)
        raise NotImplementedError
      end

      # Refill the buffer from the file.
      def refill
        start = @buffer_start + @buffer_position
        last = start + BUFFER_SIZE
        if (last > length())               # don't read past EOF
          last = length()
        end
        @buffer_length = last - start
        if (@buffer_length <= 0)
          raise EOFError
        end

        if (@buffer == nil)
          @buffer = BUFFER.clone # allocate buffer lazily
        end
        read_internal(@buffer, 0, @buffer_length)

        @buffer_start = start
        @buffer_position = 0
      end
  end
end
