module Ferret::Index
  class SegmentTermEnum < TermEnum

    INT_MAX = (2**31)-1

    attr_reader :field_infos, :size, :position, :index_pointer,
      :index_interval, :skip_interval

    def initialize(input, field_infos, is_index)
           
      @input = input
      @field_infos = field_infos
      @is_index = is_index
      @position = -1

      @term_buffer = TermBuffer.new()
      @prev_buffer = TermBuffer.new()
      @scratch = nil # used for scanning
      @term_info = TermInfo.new()

      @index_pointer = 0
      @format_m1skip_interval = nil

      first_int = @input.read_int()

      if (first_int >= 0) 
        # original-format file, without explicit format version number
        @format = 0
        @size = first_int

        # back-compatible settings
        @index_interval = 128
        @skip_interval = INT_MAX # switch off skip_to optimization

      else 
        # we have a format version number
        @format = first_int

        # check that it is a format we can understand
        if (@format < TermInfosWriter::FORMAT)
          raise "Unknown format version:#{@format}"
        end

        @size = @input.read_long()                    # read the size
        
        if (@format == -1)
          if (!@is_index) 
            @index_interval = @input.read_int()
            @format_m1skip_interval = @input.read_int()
          end
          # switch off skip_to optimization for file format prior to
          # 1.4rc2 in order to avoid a bug in skip_to implementation
          # of these versions
          @skip_interval = INT_MAX
        else
          @index_interval = @input.read_int()
          @skip_interval = @input.read_int()
        end
      end
    end
    
    #attr_accessors for the clone method
    attr_accessor :input, :term_buffer, :prev_buffer
    protected :input, :input=, :term_buffer,
      :term_buffer=, :prev_buffer, :prev_buffer=

    def clone() 
      clone = super
      clone.input = @input.clone()
      clone.term_info = TermInfo.new(@term_info)
      clone.term_buffer = @term_buffer.clone
      clone.prev_buffer = @prev_buffer.clone
      return clone
    end

    def seek(pointer, position, term, term_info)
      @input.seek(pointer)
      @position = position
      @term_buffer.term = term
      @prev_buffer.reset()
      @term_info.set!(term_info)
    end

    # Increments the enumeration to the next element.  True if one exists.
    def next?
      @position += 1
      if (@position > @size - 1) 
        @term_buffer.reset()
        return false
      end

      @prev_buffer.set!(@term_buffer)

      puts "About to read from #{term_buffer}"
      @term_buffer.read(@input, @field_infos)
      if (@is_index)
        puts "after term buffer = #{@term_buffer}"
      end

      @term_info.doc_freq = @input.read_vint()          # read doc freq
      @term_info.freq_pointer += @input.read_vlong()    # read freq pointer
      @term_info.prox_pointer += @input.read_vlong()    # read prox pointer
      
      if (@format == -1)
        # just read skip_offset in order to increment  file pointer
        # value is never used since skip_to is switched off
        if (!@is_index) 
          if (@term_info.doc_freq > @format_m1skip_interval) 
            @term_info.skip_offset = @input.read_vint()
          end
        end
      else
        if (@term_info.doc_freq >= @skip_interval) 
          @term_info.skip_offset = @input.read_vint()
        end
      end
      
      if (@is_index)
        @index_pointer += @input.read_vlong() # read index pointer
      end

      return true
    end

    # Optimized scan, without allocating new terms. 
    def scan_to(term)
      if (@scratch == nil)
        @scratch = TermBuffer.new()
      end
      @scratch.term = term
      while (@scratch > @term_buffer and next?) do
      end
    end

    # Returns the current Term in the enumeration.
    # Initially invalid, valid after next() called for the first time.
    def term
      return @term_buffer.to_term()
    end

    # Returns the previous Term enumerated. Initially nil.
    def prev
      return @prev_buffer.to_term()
    end

    # Returns the current TermInfo in the enumeration.
    # Initially invalid, valid after next() called for the first time.
    def term_info
      return @term_info.clone
    end

    # Sets the argument to the current TermInfo in the enumeration.
    # Initially invalid, valid after next() called for the first time.
    def term_info=(ti) 
      return ti.set!(term_info)
    end

    # Returns the doc_freq from the current TermInfo in the enumeration.
    # Initially invalid, valid after next() called for the first time.
    def doc_freq
      return term_info.doc_freq
    end

    # Returns the freq_pointer from the current TermInfo in the enumeration.
    # Initially invalid, valid after next() called for the first time.
    def freq_pointer
      return term_info.freq_pointer
    end

    # Returns the prox_pointer from the current TermInfo in the enumeration.
    # Initially invalid, valid after next() called for the first time.
    def prox_pointer
      return term_info.prox_pointer
    end

    # Closes the enumeration to further activity, freeing resources. 
    def close
      @input.close()
    end
  end
end
