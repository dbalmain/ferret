module Ferret::Index
  class TermBuffer
    include Comparable

    attr_reader :text_buf, :text_length, :field

    def initialize
      @text_buf = String.new
      @text_length = -1
      @field = nil
    end

    def hash()
      return text.hash + @field.hash
    end

    def <=>(other) 
      if (@field == other.field)
        return text <=> other.text
      end
      @field <=> other.field
    end

    def read(input, field_infos)
      @term = nil                           # invalidate cache
      start = input.read_vint()
      length = input.read_vint()
      total_length = start + length
      @text_length = total_length
      input.read_chars(@text_buf, start, length)
      @field = field_infos[input.read_vint()].name
    end

    def term=(term) 
      if (term == nil) 
        reset()
        return
      end

      # copy text into the buffer
      @text_buf = term.text.clone
      @text_length = @text_buf.length

      @field = term.field
      @term = term
    end

    def set!(other) 
      @text_length = other.text_length
      @text_buf = other.text_buf.clone if other.text_buf
      @field = other.field
      @term = other.term
    end
    alias :initialize_copy :set!

    def reset() 
      @field = nil
      @text_buf = ""
      @text_length = 0
      @term = nil
    end

    def to_term() 
      if @field.nil?                            # unset
        return nil
      end

      if @term.nil?
        @term = Term.new(@field, @text_buf[0,@text_length].to_s)
      end
      return @term
    end
    alias :term :to_term

    def text()
      @text_buf[0,@text_length]
    end

    def to_s()
      to_term.to_s
    end
  end
end
