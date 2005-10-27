module Ferret::Index
  class TermBuffer
    include Comparable

    attr_reader :text, :text_length, :field

    def initialize
      @text = String.new
      @text_length = -1
      @field = nil
    end

    def hash()
      return @text.hash + @field.hash
    end

    def <=>(other) 
      if (@field == other.field)
        return text_str <=> other.text_str
      end
      @field <=> other.field
    end

    def read(input, field_infos)
      @term = nil                           # invalidate cache
      start = input.read_vint()
      length = input.read_vint()
      total_length = start + length
      @text_length = total_length
      input.read_chars(@text, start, length)
      @field = field_infos[input.read_vint()].name
    end

    def term=(term) 
      if (term == nil) 
        reset()
        return
      end

      # copy text into the buffer
      @text_length = term.text.length
      @text = term.text.clone

      @field = term.field
      @term = term
    end

    def set!(other) 
      @text_length = other.text_length
      @text = other.text.clone if other.text
      @field = other.field
      @term = other.term
    end

    def reset() 
      @field = nil
      @text = String.new
      @text_length = 0
      @term = nil
    end

    def to_term() 
      if @field.nil?                            # unset
        return nil
      end

      if @term.nil?
        @term = Term.new(@field, @text[0,@text_length].to_s)
      end
      return @term
    end
    alias :term :to_term

    def initialize_copy(o)
      set!(o)
    end

    def text_str()
      @text[0,@text_length]
    end

    def to_s()
      to_term.to_s
    end
  end
end
