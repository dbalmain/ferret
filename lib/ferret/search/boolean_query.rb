module Ferret::Search
  # A Query that matches documents matching boolean combinations of other
  # queries, e.g. TermQuerys, PhraseQuerys or other BooleanQuerys.
  class BooleanQuery < Query 
    
    # The maximum number of clauses permitted. Default value is 1024.
    #
    # TermQuery clauses are generated from for example prefix queries and
    # fuzzy queries. Each TermQuery needs some buffer space during search,
    # so this parameter indirectly controls the maximum buffer requirements
    # for query search.
    #
    # When this parameter becomes a bottleneck for a Query one can use a
    # Filter. For example instead of a RangeQuery one can use a RangeFilter.
    #
    # Attempts to add more than the permitted number of clauses cause
    # TooManyClauses to be raisen.
    attr_accessor :max_clause_count
    attr_accessor :clauses
    DEFAULT_MAX_CLAUSE_COUNT = 1024

    @@max_clause_count = DEFAULT_MAX_CLAUSE_COUNT
    def BooleanQuery.max_clause_count
      return @@max_clause_count
    end
    def BooleanQuery.max_clause_count=(mcc)
      @@max_clause_count = mcc
    end

    # Thrown when an attempt is made to add more than #max_clause_count()
    # clauses. This typically happens if a PrefixQuery, FuzzyQuery,
    # WildcardQuery, or RangeQuery is expanded to many terms during search. 
    class TooManyClauses < Exception
    end

    # Constructs an empty boolean query.
    # 
    # Similarity#coord(int,int) may be disabled in scoring, as appropriate.
    # For example, this score factor does not make sense for most automatically
    # generated queries, like WildcardQuery and FuzzyQuery.
    # 
    # coord_disabled:: disables Similarity#coord(int,int) in scoring.
    def initialize(coord_disabled = false) 
      super()
      @coord_disabled = coord_disabled
      @clauses = []
    end

    # Returns true iff Similarity#coord(int,int) is disabled in scoring for
    # this query instance.
    # See #BooleanQuery(boolean)
    def coord_disabled?()
      return @coord_disabled
    end

    def similarity(searcher) 
      sim = super
      if (@coord_disabled) # disable coord as requested
        class <<sim 
          def coord(overlap, max_overlap) 
            return 1.0
          end
        end
      end
      return sim
    end

    # Adds a clause to a boolean query.  Clauses may be:
    # 
    # required::   which means that documents which _do not_ match this
    #              sub-query will _not_ match the boolean query
    # prohibited:: which means that documents which _do_ match this
    #              sub-query will _not_ match the boolean query; or
    # neither::    in which case matched documents are neither prohibited
    #              from nor required to match the sub-query. However, a
    #              document must match at least 1 sub-query to match the
    #              boolean query.
    # 
    # * For +required+ use add(query, BooleanClause::Occur::MUST)
    # * For +prohibited+ use add(query, BooleanClause::Occur::MUST_NOT)
    # * For +neither+ use add(query, BooleanClause::Occur::SHOULD)
    # 
    # raises:: TooManyClauses if the new number of clauses exceeds the
    #          maximum clause number #max_clause_count()
    def add_query(query, occur=BooleanClause::Occur::SHOULD) 
      add_clause(BooleanClause.new(query, occur))
    end

    # Adds a clause to a boolean query.
    # raises:: TooManyClauses if the new number of clauses exceeds the
    #          maximum clause number. See #max_clause_count()
    def add_clause(clause) 
      if @clauses.size >= @@max_clause_count
        raise TooManyClauses
      end

      @clauses << clause
      self
    end
    alias :<< :add_clause

    class BooleanWeight < Weight 
      attr_accessor :similarity
      attr_accessor :weights
      attr_reader :query

      def initialize(query, searcher)
        @query = query
        @weights = []
       
        @similarity = query.similarity(searcher)
        query.clauses.each do |clause|
          @weights << clause.query.create_weight(searcher)
        end
      end

      def value()
        return @query.boost()
      end

      def sum_of_squared_weights()
        sum = 0
        @weights.each_with_index do |weight, i|
          clause = @query.clauses[i]
          if not clause.prohibited?
            sum += weight.sum_of_squared_weights()         # sum sub weights
          end
        end

        sum *= @query.boost() * @query.boost()             # boost each sub-weight

        return sum 
      end


      def normalize(norm) 
        norm *= @query.boost()
        @weights.each_with_index do |weight, i|
          clause = @query.clauses[i]
          if not clause.prohibited?
            weight.normalize(norm)
          end
        end
      end

      # returns:: An alternative Scorer that uses and provides skip_to(),
      # and scores documents in document number order.
      def scorer(reader)
        result = BooleanScorer.new(@similarity)

        @weights.each_with_index do |weight, i|
          clause = @query.clauses[i]
          sub_scorer = weight.scorer(reader)
          if (sub_scorer != nil)
            result.add_scorer(sub_scorer, clause.occur)
          elsif (clause.required?())
            return nil
          end
        end

        return result
      end

      def explain(reader, doc)
       
        sum_expl = Explanation.new()
        sum_expl.description = "sum of:"
        coord = 0
        max_coord = 0
        sum = 0.0

        @weights.each_with_index do |weight, i|
          clause = @query.clauses[i]
          explanation = weight.explain(reader, doc)
          max_coord += 1 if not clause.prohibited?
          if explanation.value > 0 
            if not clause.prohibited?
              sum_expl << explanation
              sum += explanation.value
              coord += 1
            else 
              return Explanation.new(0.0, "match prohibited")
            end
          elsif clause.required?
            return Explanation.new(0.0, "match required")
          end
        end
        sum_expl.value = sum

        if (coord == 1)                          # only one clause matched
          sum_expl = sum_expl.details[0]         # eliminate wrapper
        end

        coord_factor = @similarity.coord(coord, max_coord)
        if (coord_factor == 1.0)                # coord is no-op
          return sum_expl                       # eliminate wrapper
        else 
          result = Explanation.new()
          result.description = "product of:"
          result << sum_expl
          result << Explanation.new(coord_factor, "coord(#{coord}/#{max_coord})")
          result.value = sum * coord_factor
          return result
        end
      end
    end #end BooleanWeight

    def create_weight(searcher)
      return BooleanWeight.new(self, searcher)
    end

    def rewrite(reader)
      if @clauses.size == 1 # optimize 1-clause queries
        clause = @clauses[0]
        if not clause.prohibited? # just return clause

          query = clause.query.rewrite(reader) # rewrite first

          if boost() != 1.0 # incorporate boost
            if query == clause.query # if rewrite was no-op
              query = query.clone    # then clone before boost
            end
            query.boost = boost() * query.boost()
          end

          return query
        end
      end

      clone = nil # recursively rewrite
      @clauses.each_with_index do |clause, i|
        query = clause.query().rewrite(reader)
        if query != clause.query() # clause rewrote: must clone
          clone ||= clone()
          clone.clauses[i] = BooleanClause.new(query, clause.occur)
        end
      end
      if (clone != nil) 
        return clone # some clauses rewrote
      else
        return self  # no clauses rewrote
      end
    end

    def extract_terms(terms) 
      @clauses.each do |clause|
        clause.query.extract_terms(terms)
      end
    end

    def initialize_copy(o)
      super
      @clauses = o.clauses.clone
    end

    # Prints a user-readable version of this query. 
    def to_s(field = nil) 
      buffer = ""
      buffer << "(" if boost != 1.0

      @clauses.each_with_index do |clause, i|
        if clause.prohibited?
          buffer << "-"
        elsif clause.required?
          buffer << "+"
        end

        sub_query = clause.query
        if sub_query.instance_of? BooleanQuery # wrap sub-bools in parens
          buffer << "(#{clause.query.to_s(field)})"
        else
          buffer << clause.query.to_s(field)
        end

        if i != (@clauses.size - 1)
          buffer << " "
        end
      end

      buffer << ")^#{boost}" if boost() != 1.0 

      return buffer
    end

    # Returns true iff +o+ is equal to this. 
    def eql?(other) 
      if not other.instance_of?(BooleanQuery)
        return false
      end
      return (boost() == other.boost() and @clauses == other.clauses)
    end
    alias :== :eql?

    # Returns a hash code value for this object.
    def hash() 
      return boost().hash ^ @clauses.hash
    end
  end
end
