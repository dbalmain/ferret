module Ferret::Search
  # Implements the fuzzy search query. The similiarity measurement
  # is based on the Levenshtein (distance) algorithm.
  class FuzzyQuery < MultiTermQuery 
    @@default_min_similarity = 0.5
    @@default_prefix_length = 0
    
    def FuzzyQuery.default_min_similarity()
      return @@default_min_similarity
    end

    def FuzzyQuery.default_min_similarity=(minimum_similarity)
      if (minimum_similarity >= 1.0)
        raise ArgumentError, "minimum_similarity cannot be greater than or equal to 1"
      elsif (minimum_similarity < 0.0)
        raise ArgumentError, "minimum_similarity cannot be less than 0"
      end
      @@default_min_similarity = minimum_similarity
    end
   
    def FuzzyQuery.default_prefix_length()
      return @@default_prefix_length
    end

    def FuzzyQuery.default_prefix_length=(prefix_length)
      if (prefix_length < 0)
        raise ArgumentError, "prefix_length cannot be less than 0"
      end
      @@default_prefix_length = prefix_length
    end
   
   
    attr_reader :prefix_length, :minimum_similarity
    # Create a new FuzzyQuery that will match terms with a similarity 
    # of at least +minimum_similarity+ to +term+.
    # If a +prefix_length+ > 0 is specified, a common prefix
    # of that length is also required.
    # 
    # term::               the term to search for
    # minimum_similarity:: a value between 0 and 1 to set the required
    #                      similarity between the query term and the matching
    #                      terms. For example, for a +minimum_similarity+ of
    #                      <tt>0.5</tt> a term of the same length as the query
    #                      term is considered similar to the query term if the
    #                      edit distance between both terms is less than
    #                      <tt>length(term)*0.5</tt>
    # prefix_length::      length of common (non-fuzzy) prefix. This is the
    #                      number of characters at the start of a term that
    #                      must be identical (fuzzy) to the query term if the
    #                      query is to match that term. 
    # raises::             ArgumentError if minimum_similarity is >= 1 or < 0
    #                      or if prefix_length < 0
    def initialize(term,
                   minimum_similarity = @@default_min_similarity,
                   prefix_length = @@default_prefix_length)
      super(term)
      
      if (minimum_similarity >= 1.0)
        raise ArgumentError, "minimum_similarity >= 1"
      elsif (minimum_similarity < 0.0)
        raise ArgumentError, "minimum_similarity < 0"
      end

      if (prefix_length < 0)
        raise ArgumentError, "prefix_length < 0"
      end
      
      @minimum_similarity = minimum_similarity
      @prefix_length = prefix_length
    end
    
    def get_term_enum(reader)
      return FuzzyTermEnum.new(reader, @term, @minimum_similarity, @prefix_length)
    end
    
    def rewrite(reader)

      fuzzy_enum = get_term_enum(reader)
      max_clause_count = BooleanQuery.max_clause_count
      st_queue = ScoreTermQueue.new(max_clause_count)

      begin 
        begin 
          min_score = 0.0
          score = 0.0
          t = fuzzy_enum.term()
          if t
            score = fuzzy_enum.difference()

            # terms come in alphabetical order, therefore if queue is full and score
            # not bigger than min_score, we can skip
            if(st_queue.size < max_clause_count or score > min_score)
              st_queue.insert(ScoreTerm.new(t, score))
              min_score = st_queue.top.score # maintain min_score
            end
          end
        end while fuzzy_enum.next?
      ensure 
        fuzzy_enum.close()
      end
      
      bq = BooleanQuery.new(true)
      st_queue.size.times do |i|
        st = st_queue.pop()
        tq = TermQuery.new(st.term)                     # found a match
        tq.boost = boost() * st.score                   # set the boost
        bq.add_query(tq, BooleanClause::Occur::SHOULD)  # add to query
      end

      return bq
    end
      
    def to_s(field = nil) 
      buffer = ""
      buffer << "#{@term.field}:" if @term.field != field
      buffer << "#{@term.text}~#{minimum_similarity}"
      buffer << "^#{boost()}" if (boost() != 1.0) 
      return buffer
    end
    
    class ScoreTerm
      attr_accessor :term, :score
      
      def initialize(term, score)
        @term = term
        @score = score
      end
    end
    
    class ScoreTermQueue < PriorityQueue 
      
      # See PriorityQueue#less_than(o1, o2)
      def less_than(st1, st2) 
        if (st1.score == st1.score)
          return st1.term > st2.term
        else
          return st1.score < st2.score
        end
      end
    end

    def eql?(o) 
      return (o.instance_of?(FuzzyQuery) and super(o) and
              (@minimum_similarity == o.minimum_similarity) and
              (@prefix_length == fuzzyQuery.prefix_length))
    end
    alias :== :eql?

    def hash() 
      return super ^ @minimum_similarity.hash ^ @prefix_length.hash
    end
  end
end
