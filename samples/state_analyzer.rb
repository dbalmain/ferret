require 'ferret'

module Ferret::Analysis
  class TokenFilter < TokenStream
    protected
      # Construct a token stream filtering the given input.
      def initialize(input)
        @input = input
      end
  end
 
  class StateFilter < TokenFilter
    STATES = {
      "nsw" => "new south wales",
      "vic" => "victoria",
      "qld" => "queensland",
      "tas" => "tasmania",
      "sa" => "south australia",
      "wa" => "western australia",
      "nt" => "northern territory",
      "act" => "australian capital territory"
    }
    def next()
      if @state
        t = @state
        @state = nil
        return t
      end
      t = @input.next
      return nil if t.nil?
      if STATES[t.text]
        @state = Token.new(t.text.upcase, t.start_offset, t.end_offset, 0)
      end
      return t
    end
  end

  class StateAnalyzer < StandardAnalyzer
    def token_stream(field, text)
      StateFilter.new(super)
    end
  end

  analyzer = StateAnalyzer.new

  ts = analyzer.token_stream(nil,
    "I used to live in NSW but now I live in the A.C.T.")

  while t = ts.next
    puts t
  end

end
