module Ferret::Analysis
  # A TokenStream enumerates the sequence of tokens, either from
  # fields of a document or from query text.
  #
  # This is an abstract class.  Concrete subclasses are:
  # * Tokenizer, a TokenStream whose input is a Reader; and
  # * TokenFilter, a TokenStream whose input is another TokenStream.
  class TokenStream
    # Returns the next token in the stream, or null at EOS.
    def next
      raise NotImplementedError
    end

    # Releases resources associated with this stream.
    def close
      raise NotImplementedError
    end

    # Iterates through the tokens in the field
    def each # :yields: token
      while (n = self.next())
        yield n
      end
    end
  end
end
