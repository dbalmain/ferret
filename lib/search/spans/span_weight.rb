module Ferret::Search::Spans
  class SpanWeight < Weight
    def initialize(query, searcher)
      @similarity = query.similarity(searcher)
      @query = query
      @terms = query.terms()

      @idf = @query.similarity(searcher).idf_phrase(@terms, searcher)
    end

    attr_reader :query, :value

    def sum_of_squared_weights()
      @query_weight = @idf * @query.boost()         # compute query weight
      return @query_weight * @query_weight             # square it
    end

    def normalize(query_norm) 
      @query_norm = query_norm
      @query_weight *= query_norm                     # normalize query weight
      @value = @query_weight * @idf                    # idf for document
    end

    def scorer(reader)
      return SpanScorer.new(@query.spans(reader), self,
                            @similarity,
                            reader.get_norms(@query.field))
    end

    def explain(reader, doc)
      result = Explanation.new()
      result.description = "weight(#{@query} in #{doc}), product of:"
      field = @query.field

      doc_freqs = @terms.map {|t| "#{t.text}=#{reader.doc_freq(t)}"}.join(' ')

      idf_expl = Explanation.new(@idf, "idf(#{field}: #{doc_freqs})")

      # explain query weight
      query_expl = Explanation.new()
      query_expl.description = "query_weight(#{@query}), product of:"

      boost_expl = Explanation.new(@query.boost, "boost")
      query_expl << boost_expl if (@query.boost != 1.0)
      query_expl << idf_expl

      query_norm_expl = Explanation.new(@query_norm,"query_norm")
      query_expl << query_norm_expl

      query_expl.value = boost_expl.value * idf_expl.value * query_norm_expl.value

      result << query_expl

      # explain field weight
      field_expl = Explanation.new()
      field_expl.description = "field_weight(#{field}:#{@query.to_s(field)}"+
                               " in #{doc}), product of:"

      tf_expl = scorer(reader).explain(doc)
      field_expl << tf_expl
      field_expl << idf_expl

      field_norm_expl = Explanation.new()
      field_norms = reader.get_norms(field)
      field_norm = (field_norms ? Similarity.decode_norm(field_norms[doc]) : 0.0)
      field_norm_expl.value = field_norm
      field_norm_expl.description = "field_norm(field=#{field}, doc=#{doc})"
      field_expl << field_norm_expl

      field_expl.value = tf_expl.value * idf_expl.value * field_norm_expl.value

      result << field_expl

      # combine them
      result.value = query_expl.value * field_expl.value

      if (query_expl.value == 1.0)
        return field_expl
      end
      return result
    end
  end
end
