module Ferret::Search
  # The abstract base class for queries.
  # Instantiable subclasses are:
  #    * TermQuery
  #    * MultiTermQuery
  #    * BooleanQuery
  #    * WildcardQuery
  #    * PhraseQuery
  #    * PrefixQuery
  #    * MultiPhraseQuery
  #    * FuzzyQuery
  #    * RangeQuery
  #    * Span::SpanQuery
  #
  # A parser for queries is contained in:
  #    * Ferret::QueryParser::QueryParser
  #
  class Query
    # documents matching this query clause will (in addition to the normal
    # weightings) have their score multiplied by the boost factor. It is
    # 1.0 be default.
    attr_accessor :boost

    def initialize()
      @boost = 1.0
    end

    # Prints a query to a string, with +field+ as the default field for
    # terms.  The representation used is one that is supposed to be readable
    # by Ferret::QueryParser::QueryParser. However, there are the following
    # limitations:
    # * If the query was created by the parser, the printed representation
    #   may not be exactly what was parsed. For example, characters that need
    #   to be escaped will be represented without the required backslash.
    # * Some of the more complicated queries (e.g. span queries)
    #   don't have a representation that can be parsed by QueryParser.
    def to_s(field=nil)
      raise NotImplementedError
    end

    # Expert: Constructs an appropriate Weight implementation for this query.
    # 
    # Only implemented by primitive queries, which re-write to themselves.
    def create_weight(searcher)
      raise NotImplementedError
    end

    # Expert: Constructs and initializes a Weight for a top-level query. 
    def weight(searcher)
      query = searcher.rewrite(self)
      weight = query.create_weight(searcher)
      sum = weight.sum_of_squared_weights()
      norm = similarity(searcher).query_norm(sum)
      weight.normalize(norm)
      return weight
    end

    # Expert: called to re-write queries into primitive queries. 
    def rewrite(reader)
      return self
    end

    # Expert: called when re-writing queries under MultiSearcher.
    # 
    # Create a single query suitable for use by all subsearchers (in 1-1
    # correspondence with queries). This is an optimization of the OR of
    # all queries. We handle the common optimization cases of equal
    # queries and overlapping clauses of boolean OR queries (as generated
    # by MultiTermQuery.rewrite() and RangeQuery.rewrite()).
    # Be careful overriding this method as queries[0] determines which
    # method will be called and is not necessarily of the same type as
    # the other queries.
    def combine(queries) 
      uniques = Set.new
      queries.each { |query|
        clauses = []
        # check if we can split the query into clauses
        splittable = query.respond_to? :clauses
        if splittable
          splittable = query.coord_disabled?
          clauses = query.clauses
          clauses.each { |clause|
            splittable = clause.occur == BooleanClause::Occur::SHOULD
            break unless splittable
          }
        end
        if splittable
          clauses.each { |clause|
            uniques << clause.query
          }
        else
          uniques << query
        end
      }
      # optimization: if we have just one query, just return it
      if uniques.size == 1
        uniques.each { |query| return query }
      end
      
      result = BooleanQuery.new(true)
      uniques.each { |query|
        result.add_query(query, BooleanClause::Occur::SHOULD)
      }
      return result
    end

    # Expert: adds all terms occuring in this query to the terms set
    def extract_terms(terms) 
      raise NotImplementedError
    end


    # Expert: merges the clauses of a set of BooleanQuery's into a single
    # BooleanQuery.
    # 
    # A utility for use by #combine() implementations.
    def merge_boolean_queries(queries) 
      all_clauses = Set.new
      queries.each do |query|
        query.clauses.each do |clause|
          all_clauses << clause
        end
      end

      coord_disabled = queries.size==0 ? false : queries[0].coord_disabled?
      result = BooleanQuery.new(coord_disabled)
      all_clauses.each do |clause|
        result << clause
      end
      return result
    end

    # Expert: Returns the Similarity implementation to be used for this
    # query.  Subclasses may override this method to specify their own
    # Similarity implementation, perhaps one that delegates through that of
    # the Searcher.  By default the Searcher's Similarity implementation is
    # returned.
    def similarity(searcher) 
      return searcher.similarity
    end
  end
end
