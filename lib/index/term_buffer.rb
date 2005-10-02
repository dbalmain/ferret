module Ferret::Index
  class TermBuffer
    include Comparable

    attr_reader :text, :text_length, :field_name

    def initialize
      @text = String.new
      @text_length = -1
      @field_name = nil
    end

    def hash()
      return @text.hash + @field_name.hash
    end

    def <=>(other) 
      if (@field_name == other.field_name)
        return text_str <=> other.text_str
      end
      @field_name <=> other.field_name
    end

    def read(input, field_infos)
      puts "fucker"
      @term = nil                           # invalidate cache
      start = input.read_vint()
      length = input.read_vint()
      total_length = start + length
      @text_length = total_length
      input.read_chars(@text, start, length)
      puts "test = #{@text}"
      @field_name = field_infos[input.read_vint()].name
    end

    def term=(term) 
      if (term == nil) 
        reset()
        return
      end

      # copy text into the buffer
      @text_length = term.text.length
      @text = term.text.clone

      @field_name = term.field_name
      @term = term
    end

    def set!(other) 
      @text_length = other.text_length
      @text = other.text.clone if other.text
      @field_name = other.field_name
      @term = other.term
    end

    def reset() 
      @field_name = nil
      @text = String.new
      @text_length = 0
      @term = nil
    end

    def to_term() 
      if @field_name.nil?                            # unset
        return nil
      end

      if @term.nil?
        @term = Term.new(@field_name, @text[0,@text_length].to_s)
      end
      return @term
    end
    alias :term :to_term

    def clone() 
      clone = TermBuffer.new()
      clone.set!(self)
      return clone
    end
    
    def text_str()
      @text[0,@text_length]
    end

    def to_s()
      to_term.to_s
    end
  end
end
