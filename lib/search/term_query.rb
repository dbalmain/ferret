module Ferret::Search
  # A Query that matches documents containing a @term.
  #  This may be combined with other terms with a BooleanQuery.
  class TermQuery < Query 

    attr_reader :term

    class TermWeight < Weight 
      attr_reader :value, :query

      def initialize(query, searcher)
        @similarity = query.similarity(searcher)
        @idf = @similarity.idf(searcher.doc_freq(query.term),
                               searcher.max_doc) # compute idf
        @query = query
        @value = 0
      end

      def to_s() return "TermWeight(#{@value})"; end

      def sum_of_squared_weights() 
        @query_weight = @idf * @query.boost()       # compute query weight
        return @query_weight * @query_weight # square it
      end

      def normalize(query_norm) 
        @query_norm = query_norm
        @query_weight *= query_norm   # normalize query weight
        @value = @query_weight * @idf # idf for document
      end

      def scorer(reader)
        term_docs = reader.term_docs_for(@query.term)

        return nil if term_docs.nil?

        return TermScorer.new(self, term_docs, @similarity,
                              reader.get_norms(@query.term.field))
      end

      def explain(reader, doc)
        explanation = Explanation.new()
        explanation.description = "weight(#{@query} in #{doc}), product of:"

        idf_expl = Explanation.new(@idf, "idf(doc_freq=#{reader.doc_freq(@query.term)})")

        # explain query weight
        query_expl = Explanation.new(nil, "query_weight(#{@query}), product of:")

        boost_expl = Explanation.new(@query.boost(), "boost")
        if (@query.boost() != 1.0)
          query_expl << boost_expl
        end
        query_expl << idf_expl

        query_norm_expl = Explanation.new(@query_norm||0.0,"query_norm")
        query_expl << query_norm_expl

        query_expl.value = boost_expl.value * idf_expl.value * query_norm_expl.value

        explanation << query_expl

        # explain field weight
        field_name = @query.term.field
        field_expl = Explanation.new()
        field_expl.description = "field_weight(#{@query.term} in #{doc}), product of:"

        tf_expl = scorer(reader).explain(doc)
        field_expl << (tf_expl)
        field_expl << (idf_expl)

        field_norms = reader.get_norms(field_name)
        field_norm = field_norms.nil? ? 0.0 : Similarity.decode_norm(field_norms[doc])
        field_norm_expl = Explanation.new(field_norm,
                                          "field_norm(field=#{field_name}, doc=#{doc})")
        field_expl << field_norm_expl

        field_expl.value = tf_expl.value * idf_expl.value * field_norm_expl.value 
        explanation << field_expl

        # combine them
        explanation.value = (query_expl.value * field_expl.value)

        if (query_expl.value == 1.0)
          return field_expl
        end

        return explanation
      end
    end

    # Constructs a query for the @query.term +t+. 
    def initialize(t) 
      super()
      @term = t
    end

    def create_weight(searcher)
      return TermWeight.new(self, searcher)
    end

    def extract_terms(terms) 
      terms << @term
    end

    # Prints a user-readable version of this query. 
    def to_s() 
      buffer = "#{@term.field}:#{@term.text}"
      if @boost != 1.0
        buffer << "^ #{boost()}"
      end
      return buffer
    end

    # Returns true iff +o+ is equal to this. 
    def eql?(other) 
      return false if not other.instance_of?(TermQuery)
      return (@boost == other.boost and @term == other.term)
    end
    alias :== :eql?

    # Returns a hash code value for this object.
    def hash() 
      return @boost.hash ^ @term.hash
    end

  end
end
