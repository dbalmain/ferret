if __FILE__ == $0
  module Ferret
  end
  $:.unshift File.dirname(__FILE__)
  require 'token_stream'
  require 'tokenizers'
  require 'token'
end

module Ferret::Analysis
  # The standard tokenizer is an advanced tokenizer which tokenizes morst
  # words correctly as well as tokenizing things like email addresses, web
  # addresses, phone numbers, etc.

  class StandardTokenizer < RETokenizer
    ALPHA      = /[[:alpha:]]+/
    APOSTROPHE = /#{ALPHA}('#{ALPHA})+/
    ACRONYM    = /#{ALPHA}\.(#{ALPHA}\.)+/
    P          = /[_\/.,-]/
    HASDIGIT   = /\w*\d\w*/

    protected

      # Collects only characters which are not spaces tabs or carraige returns
      def token_re()
        #/#{NUM}|#{EMAIL}|#{ACRONYM}\w*|#{C0MPANY}|#{APOSTROPHE}|\w+/
        # This is a simplified version of the original Lucene standard
        # tokenizer.  I think it works better. I hope so anyway. Any way to
        # do this more neatly?
        /[[:alpha:]]+(('[[:alpha:]]+)+
                     |\.([[:alpha:]]\.)+
                     |(@|\&)\w+([-.]\w+)*
                     )
        |\w+(([\-._]\w+)*\@\w+([-.]\w+)+
            |#{P}#{HASDIGIT}(#{P}\w+#{P}#{HASDIGIT})*(#{P}\w+)?
            |(\.\w+)+
            |
            )
        /x
      end

      # stem the 's and remove the '.'s from acronyms
      def normalize(str)
        if str =~ /^#{ACRONYM}$/
          str.gsub!(/\./, '')
        elsif str =~ /^#{APOSTROPHE}$/
          str.gsub!(/'[sS]$/, '')
        end
        str
      end
  end
end

# Add this so we can play around with the standard tokenizer
if __FILE__ == $0
  st = "\033[7m"
  en = "\033[m"

  $stdin.each do |line|
    stk = Ferret::Analysis::StandardTokenizer.new(line)
    while tk = stk.next()
      puts "    <" + tk.term_text + "> from " + tk.start_offset.to_s + " to " + tk.end_offset.to_s
    end
  end
end
