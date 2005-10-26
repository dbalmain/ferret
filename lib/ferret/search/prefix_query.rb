module Ferret::Search
  # A Query that matches documents containing terms with a specified prefix. A
  # PrefixQuery is built by QueryParser for input like +app*+. 
  class PrefixQuery < Query 
    attr_reader :prefix
    # Constructs a query for terms starting with +prefix+. 
    def initialize(prefix) 
      super()
      @prefix = prefix
    end

    def rewrite(reader)
      bq = BooleanQuery.new(true)
      enumerator = reader.terms_from(@prefix)
      begin 
        prefix_text = @prefix.text
        prefix_length = prefix_text.length
        prefix_field = @prefix.field
        begin 
          term = enumerator.term
          if (term.nil? or
            term.field != prefix_field or
            term.text[0,prefix_length] != prefix_text)
            break
          end
            tq = TermQuery.new(term)                        # found a match
            tq.boost = boost()                              # set the boost
            bq.add_query(tq, BooleanClause::Occur::SHOULD)  # add to query
            #puts("added " + term)
        end while (enumerator.next?)
      ensure 
        enumerator.close()
      end
      return bq
    end

    # Prints a user-readable version of this query. 
    def to_s(f) 
      buffer = ""
      buffer << "#{@prefix.field}:" if @prefix.field != f
      buffer << "#{@prefix.text}*"
      buffer << "^#{boost()}" if boost() != 1.0 
      return buffer
    end

    def eql?(o)
      (@prefix == o.prefix and boost() == o.boost)
    end

    def hash()
      boost().hash ^ @prefix.hash
    end
  end
end
