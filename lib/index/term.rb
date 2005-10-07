module Ferret::Index
  # A Term represents a word from text.  This is the unit of search.  It is
  # composed of two elements, the text of the word, as a string, and the name of
  # the field that the text occured in, an interned string.
  #
  # Note that terms may represent more than words from text fields, but also
  # things like dates, email addresses, urls, etc.
  #
  # A term contains two attributes;
  # field:: The field indicates the part of a document which this term came from.
  # text::  In the case of words, this is simply the text of the word.  In the case
  #         of dates and other types, this is an encoding of the object as a string.
  class Term
    include Comparable

    attr_accessor :field
    attr_accessor :text

    # Constructs a Term with the given field and text
    def initialize(fld_name, txt)
      @field = fld_name
      @text = txt
    end

    # Combines the hash() of the field and the text.
    def hash()
      return field.hash() + text.hash()
    end

    # implements comparable giving us the methods >, >=, <, <= and between? 
    def <=>(other)
      if @field == other.field 
        return @text <=> other.text 
      else
        return @field <=> other.field
      end
    end
    alias :eql? :==

    # Resets the field and text of a Term.
    def set!(fld_name, txt)
      initialize(fld_name, txt)
    end

    def to_s
      @field + ":" + @text
    end
  end
end
