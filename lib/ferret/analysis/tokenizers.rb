require 'strscan'

module Ferret::Analysis
  # A Tokenizer is a TokenStream whose input is a Reader.
  #
  # This is an abstract class.
  class Tokenizer < TokenStream
    # By default, closes the input Reader.
    def close()
      @input.close()
    end

    protected
      # Construct a token stream processing the given input.
      def initialize(input)
        @input = input
      end
  end

  # An abstract base class for simple regular expression oriented
  # tokenizers. Very powerful tokenizers can be created using this class as
  # can be seen from the StandardTokenizer class. Bellow is an example of a
  # simple implementation of a LetterTokenizer using an RegExpTokenizer.
  # Basically, a token is a sequence of alphabetic characters separated by
  # one or more non-alphabetic characters.
  #
  #   class LetterTokenizer < RegExpTokenizer
  #       def token_re()
  #         /[[:alpha:]]+/
  #       end
  #   end
  class RegExpTokenizer < Tokenizer

    # Initialize with an IO implementing input such as a file.
    #
    # input:: must have a read(count) method which returns an array or string
    #         of _count_ chars.
    def initialize(input)
      if input.is_a? String
        @ss = StringScanner.new(input)
      else
        @ss = StringScanner.new(input.read())
      end
    end

    # Returns the next token in the stream, or null at EOS.
    def next()
      if @ss.scan_until(token_re)
        term = @ss.matched
        term_end = @ss.pos
        term_start = term_end - term.size
      else
        return nil
      end

      return Token.new(normalize(term), term_start, term_end)
    end

    def close()
      @ss = nil
    end

    protected
      # returns the regular expression used to find the next token
      def token_re
        /[[:alpha:]]+/
      end

      # Called on each token to normalize it before it is added to the
      # token.  The default implementation does nothing.  Subclasses may
      # use this to, e.g., lowercase tokens.
      def normalize(str) return str end
  end

  
  # A LetterTokenizer is a tokenizer that divides text at non-letters.
  # That's to say, it defines tokens as maximal strings of adjacent letters,
  # as defined by the regular expression _/[[:alpha:]]+/_.
  class LetterTokenizer < RegExpTokenizer
    protected
      # Collects only characters which satisfy the regular expression
      # _/[[:alpha:]]+/_.
      def token_re()
        /[[:alpha:]]+/
      end
  end

  # LowerCaseTokenizer performs the function of LetterTokenizer
  # and LowerCaseFilter together.  It divides text at non-letters and converts
  # them to lower case.
  class LowerCaseTokenizer < LetterTokenizer
    protected
      def normalize(str)
        return str.downcase
      end
  end

  # A WhiteSpaceTokenizer is a tokenizer that divides text at whiteSpace.
  # Adjacent sequences of non-WhiteSpace characters form tokens.
  class WhiteSpaceTokenizer < RegExpTokenizer
    protected
      # Collects only characters which are not spaces tabs or carraige returns
      def token_re()
        /\S+/
      end
  end
end
