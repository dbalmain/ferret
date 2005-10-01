module Ferret::Index
  # A Term represents a word from text.  This is the unit of search.  It is
  # composed of two elements, the text of the word, as a string, and the name of
  # the field that the text occured in, an interned string.
  #
  # Note that terms may represent more than words from text fields, but also
  # things like dates, email addresses, urls, etc.
  #
  # A term contains two attributes;
  # field_name:: The field indicates the part of a document which this term came from.
  # text::  In the case of words, this is simply the text of the word.  In the case
  #         of dates and other types, this is an encoding of the object as a string.
  class Term
    include Comparable

    attr_accessor :field_name
    attr_accessor :text

    # Constructs a Term with the given field and text
    def initialize(fld_name, txt)
      @field_name = fld_name
      @text = txt
    end

    # Combines the hash() of the field_name and the text.
    def hash()
      return field_name.hash() + text.hash()
    end

    # implements comparable giving us the methods >, >=, <, <= and between? 
    def <=>(other)
      if @field_name == other.field_name 
        return @text <=> other.text 
      else

        return @field_name <=> other.field_name
      end
    end
    alias :eql? :==

    # Resets the field_name and text of a Term.
    def set!(fld_name, txt)
      initialize(fld_name, txt)
    end

    def to_s
      @field_name + ":" + @text
    end
  end

  # Term Data holds all the information for a terms occurances in the
  # document set. Each term should have one term data while the index is
  # being built.
  #
  class TermData
    attr_reader :documents, :positions, :frequencies, :offsets

    def initialize
      @index = {}
      @documents = []
      @positions = []
      @frequencies = []
      @offsets = []
    end

    # Adds data to the term. All of this data may not be recorded but it
    # will be stored here until the term is written to the index.
    # 
    # doc_num::      The document number for the document this term was found in.
    # position::     The position in the field that this term was found.
    # start_offset:: The start offset of the term in the document, ie. the
    #                position of the first character in the field
    # end_offset::   The end offset of the character in this document.
    def add_data(doc_num, position, start_offset, end_offset)
      i = @index[doc_num] ||= @documents.length
      @documents[i] = doc_num
      @frequencies[i] = (@frequencies[i] || 0) + 1
      (@positions[i] ||= [] ) << position
      (@offsets[i] ||= [] ) << [start_offset, end_offset]
    end

    # Find the array of positions that this term appears in the document
    def positions_in_doc(doc_num)
      return @positions[@index[doc_num]]
    end
    
    # Find the frequency that this term appears in the document
    def frequency_in_doc(doc_num)
      return @frequencies[@index[doc_num]]
    end
    
    # Find the array of offsets of this term appearances in the document
    def offsets_in_doc(doc_num)
      return @offsets[@index[doc_num]]
    end
  end
end
