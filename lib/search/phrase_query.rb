module Ferret::Search
  # A Query that matches documents containing a particular sequence of terms.
  # A PhraseQuery is built by QueryParser for input like +"new york"+.
  # 
  # This query may be combined with other terms or queries with a BooleanQuery.
  class PhraseQuery < Query 
    def initialize()
      super
      @slop = 0
      @terms = []
      @positions = []
      @field = nil
    end

    # Sets the number of other words permitted between words in query phrase.
    # If zero, then this is an exact phrase search.  For larger values this
    # works like a +WITHIN+ or +NEAR+ operator.
    #
    # The slop is in fact an edit-distance, where the units correspond to
    # moves of terms in the query phrase out of position.  For example, to
    # switch the order of two words requires two moves (the first move places
    # the words atop one another), so to permit re-orderings of phrases, the
    # slop must be at least two.
    #
    # More exact matches are scored higher than sloppier matches, thus search
    # results are sorted by exactness.
    #
    # The slop is zero by default, requiring exact matches.
    attr_accessor :slop
    attr_reader :terms, :positions, :field

    # Adds a term to the end of the query phrase.
    #
    # The relative position of the term is the one immediately after the last
    # term added, unless explicitly specified. By specifying explicitly,
    # you can have phrases with more than one term at the same position or
    # phrases with gaps (e.g. in connection with stopwords).
    #
    # term:: the term to search for
    # position:: the relative position of the term to the rest of the terms
    # int the query.
    def add(term, position = nil, pos_inc = 1) 
      if position.nil?
        position = (@positions.size > 0) ? (@positions[-1] + pos_inc) : 0
      end
      
      if @terms.size == 0
        @field = term.field
      elsif (term.field != @field)
        raise ArgumentError, "All phrase terms must be in the same field: #{term}"
      end
      
      @terms << term
      @positions << position
    end

    def <<(term)
      add(term)
      return self
    end
    
    class PhraseWeight < Weight 
      attr_reader :query, :value

      def initialize(query, searcher)
        @query = query
        @similarity = query.similarity(searcher)
        @idf = @similarity.idf_phrase(@query.terms, searcher)
      end

      def to_s() return "phrase_weight(#{@value})" end

      def sum_of_squared_weights() 
        @query_weight = @idf * @query.boost()  # compute query weight
        return @query_weight * @query_weight   # square it
      end

      def normalize(query_norm) 
        @query_norm = query_norm
        @query_weight *= query_norm            # normalize query weight
        @value = @query_weight * @idf          # idf for document 
      end

      def scorer(reader)
        return nil if @query.terms.size == 0   # optimize zero-term case

        tps = []
        @query.terms.each do |term|
          tp = reader.term_positions_for(term)
          return nil if tp.nil?
          tps << tp
        end

        if (@query.slop == 0)				  # optimize exact case
          return ExactPhraseScorer.new(self, tps, @query.positions,
                                       @similarity,
                                       reader.get_norms(@query.field))
        else
          return SloppyPhraseScorer.new(self, tps, @query.positions,
                                   @similarity,
                                   @query.slop,
                                   reader.get_norms(@query.field))
        end
      end

      def explain(reader, doc)
        result = Explanation.new()
        result.description = "weight(#{@query} in #{doc}), product of:"

        doc_freqs = @query.terms.map do |term|
          "#{term.text}=#{reader.doc_freq(term)}"
        end.join(", ")

        idf_expl = Explanation.new(@idf, "idf(#{@query.field}:<#{doc_freqs}>)")
        
        # explain query weight
        query_expl = Explanation.new()
        query_expl.description = "query_weight(#{@query}), product of:"

        boost = @query.boost()
        if boost != 1.0
          boost_expl = Explanation.new(boost, "boost")
          query_expl << boost_expl
        end
        query_expl << idf_expl
        
        query_norm_expl = Explanation.new(@query_norm, "query_norm")
        query_expl << query_norm_expl
        
        query_expl.value = boost * @idf * query_norm_expl.value

        result << query_expl
       
        # explain field weight
        field_expl = Explanation.new()
        field_expl.description =
          "field_weight(#{query} in #{doc}), product of:"

        tf_expl = scorer(reader).explain(doc)
        field_expl << tf_expl
        field_expl << idf_expl

        field_norm_expl = Explanation.new()
        field_norms = reader.get_norms(@query.field)
        field_norm =
          field_norms ? Similarity.decode_norm(field_norms[doc]) : 0.0
        field_norm_expl.value = field_norm
        field_norm_expl.description =
          "field_norm(field=#{@query.field}, doc=#{doc})"
        field_expl << field_norm_expl

        field_expl.value = tf_expl.value * @idf * field_norm
        
        result << field_expl

        # combine them
        result.value = query_expl.value * field_expl.value

        if query_expl.value == 1.0
          return field_expl
        else
          return result
        end
      end
    end

    def create_weight(searcher)
      if @terms.size == 1 # optimize one-term case
        term = @terms[0]
        tq = TermQuery.new(term)
        tq.boost = boost()
        return tq.create_weight(searcher)
      end
      return PhraseWeight.new(self, searcher)
    end

    # See Query#extract_terms()
    def extract_terms(query_terms) 
      query_terms.add_all(@terms)
    end

    # Prints a user-readable version of this query. 
    def to_s(f=nil) 
      buffer = ""
      buffer << "#{@field}:" if @field != f
      buffer << '"'
      last_pos = -1
      @terms.each_index do |i|
        term = @terms[i]
        pos = @positions[i]
        last_pos.upto(pos-2) {buffer << "<> "}
        last_pos = pos
        buffer << "#{term.text} "
      end
      buffer.rstrip!
      buffer << '"'
      buffer << "~#{slop}" if (slop != 0) 
      buffer << "^#{boost()}" if boost() != 1.0
      return buffer
    end

    # Returns true iff +o+ is equal to this. 
    def eql?(o) 
      if not o.instance_of? PhraseQuery
        return false
      end
      return (boost() == o.boost() and @slop == o.slop and
        @terms == o.terms and @positions == o.positions)
    end
    alias :== :eql?

    # Returns a hash code value for this object.
    def hash() 
      return boost().hash ^ slop.hash ^ @terms.hash ^ @positions.hash
    end
  end
end
