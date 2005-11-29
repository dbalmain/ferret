module Ferret::Analysis
  # A Token is an occurence of a term from the text of a field.  It consists
  # of a term's text, the start and end offset of the term in the text of the
  # field, and a type string.
  #
  # The start and end offsets permit applications to re-associate a token with
  # its source text, e.g., to display highlighted query terms in a document
  # browser, or to show matching text fragments in a KWIC (KeyWord In Context)
  # display, etc.
  #
  # The type is an interned string, assigned by a lexical analyzer (a.k.a.
  # tokenizer), naming the lexical or syntactic class that the token belongs
  # to.  For example an end of sentence marker token might be implemented with
  # type "eos".  The default token type is "word".
  #
  # start_offset:: is the position of the first character corresponding to
  #                this token in the source text
  # end_offset:: is equal to one greater than the position of the last
  #              character corresponding of this token Note that the
  #              difference between @end_offset and @start_offset may not be
  #              equal to @term_text.length(), as the term text may have been
  #              altered by a stemmer or some other filter.
  class Token
    include Comparable
    attr_accessor :term_text
    attr_reader :position_increment, :start_offset, :end_offset, :type

    # Constructs a Token with the given term text, and start & end offsets.
    # The type defaults to "word."
    def initialize(txt, so, eo, typ="word", pos_inc=1)
      @term_text = txt
      @start_offset = so
      @end_offset = eo
      @type = typ # lexical type
      @position_increment = pos_inc
    end

    def set!(txt, so, eo)
      @term_text = txt
      @start_offset = so
      @end_offset = eo
      self
    end

    def eql?(o)
      return (o.instance_of?(Token) and @start_offset == o.start_offset and
              @end_offset == o.end_offset and @term_text == o.term_text)
    end
    alias :== :eql?

    # Tokens are sorted by the position in the text at which they occur, ie
    # the start_offset. If two tokens have the same start offset, (see
    # position_increment=) then, they are sorted by the end_offset and then
    # lexically by the token text.
    def <=>(o)
      r = @start_offset <=> o.start_offset
      return r if r != 0
      r = @end_offset <=> o.end_offset
      return r if r != 0
      r = @term_text <=> o.term_text
      return r
    end

    # Set the position increment.  This determines the position of this token
    # relative to the previous Token in a TokenStream, used in phrase
    # searching.
    #
    # The default value is one.
    #
    # Some common uses for this are:
    #
    # * Set it to zero to put multiple terms in the same position.  This is
    #   useful if, e.g., a word has multiple stems.  Searches for phrases
    #   including either stem will match.  In this case, all but the first
    #   stem's increment should be set to zero: the increment of the first
    #   instance should be one.  Repeating a token with an increment of zero
    #   can also be used to boost the scores of matches on that token.
    #
    # * Set it to values greater than one to inhibit exact phrase matches.
    #   If, for example, one does not want phrases to match across removed
    #   stop words, then one could build a stop word filter that removes stop
    #   words and also sets the increment to the number of stop words removed
    #   before each non-stop word.  Then exact phrase queries will only match
    #   when the terms occur with no intervening stop words.
    def position_increment=(pos_inc)
      if (pos_inc < 0)
        raise ArgumentError, "Increment must be zero or greater: " + pos_inc
      end
      @position_increment = pos_inc
    end

    # Returns a string representation of the token with all the attributes.
    def to_s
      buf = "#{term_text}:#{start_offset}->#{end_offset}"
      buf << "(pos_inc=#{@position_increment})" if (@position_increment != 1)
      buf << "(type=#{@type})" if (@type != "word")
      buf
    end
  end
end
