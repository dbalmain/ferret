module Ferret::Analysis
  # A TokenFilter is a TokenStream whose input is another token stream.
  #
  # This is an abstract class.
  class TokenFilter < TokenStream
    # Close the input TokenStream.
    def close()
      @input.close()
    end

    protected
      # Construct a token stream filtering the given input.
      def initialize(input)
        @input = input
      end
  end

  # Normalizes token text to lower case.
  class LowerCaseFilter < TokenFilter
    def next()
      t = @input.next()

      if (t == nil)
        return nil
      end

      t.term_text = t.term_text.downcase()

      return t
    end
  end

  # Removes stop words from a token stream. To will need to pass your own
  # set of stopwords to use this stop filter. If you with to use the default
  # list of stopwords then use the StopAnalyzer.
  class StopFilter < TokenFilter
    # Constructs a filter which removes words from the input
    # TokenStream that are named in the array of words.
    def initialize(input, stop_set)
      super(input);
      @stop_set = stop_set
    end

    def StopFilter.new_with_file(input, path)
      ws = WordListLoader.word_set_from_file(path)
      return StopFilter.new(input, ws)
    end

    # Returns the next input Token whose termText() is not a stop word.
    def next()
      # return the first non-stop word found
      while token = @input.next()
        return token if ! @stop_set.include?(token.term_text)
      end
      return nil
    end
  end

  # Transforms the token stream as per the Porter stemming algorithm.
  # Note: the input to the stemming filter must already be in lower case,
  # so you will need to use LowerCaseFilter or LowerCaseTokenizer further
  # down the Tokenizer chain in order for this to work properly!
  # 
  # To use this filter with other analyzers, you'll want to write an
  # Analyzer class that sets up the TokenStream chain as you want it.
  # To use this with LowerCaseTokenizer, for example, you'd write an
  # analyzer like this:
  #
  #   def MyAnalyzer < Analyzer
  #     def token_stream(field, reader)
  #       return PorterStemFilter.new(LowerCaseTokenizer.new(reader))
  #     end
  #   end
  class PorterStemFilter < TokenFilter
    # Returns the next input Token, after being stemmed
    def next()
      token = @input.next()
      if (token == nil)
        return nil
      else
        token.term_text = Stemmable.stem_porter(token.term_text)
      end
      token
    end
  end
end
