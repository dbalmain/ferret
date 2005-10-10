# A query that matches all documents.
class MatchAllDocsQuery < Query 

  def initialize() 
    super
  end

  class MatchAllScorer < Scorer 

    def initialize(reader, similarity) 
      super(similarity)
      @reader = reader
      @count = -1
      @max_doc = reader.max_doc
    end

    def doc() 
      return @count
    end

    def explain(doc) 
      return Explanation.new(1.0, "MatchAllDocsQuery")
    end

    def next? 
      while (@count < (@max_doc - 1)) 
        @count += 1
        if (!@reader.deleted?(@count)) 
          return true
        end
      end
      return false
    end

    def score() 
      return 1.0
    end

    def skip_to(target) 
      @count = target - 1
      return next?
    end
  end

  class MatchAllDocsWeight < Weight 
    attr_reader :query
    def initialize(query, searcher) 
      @query = query
      @searcher = searcher
    end

    def to_s() 
      return "weight(#{@query})"
    end

    def value() 
      return 1.0
    end

    def sum_of_squared_weights() 
      return 1.0
    end

    def normalize(query_norm) 
    end

    def scorer(reader) 
      return MatchAllScorer.new(reader, @query.similarity(@searcher))
    end

    def explain(reader, doc) 
      # explain query weight
      query_expl = Explanation.new(1.0, "MatchAllDocsQuery")
      boost_expl = Explanation.new(@query.boost, "boost")
      if (boost_expl.value != 1.0)
        query_expl << boost_expl
        query_expl.value = boost_expl.value
      end

      return query_expl
    end
  end

  def create_weight(searcher) 
    return MatchAllDocsWeight.new(self, searcher)
  end

  def to_s(field) 
    buffer = "MatchAllDocsQuery"
    buffer << "^#{boost}" if (boost() != 1.0) 
    return buffer
  end

  def eql?(o) 
    return (o.instance_of?(MatchAllDocsQuery) and boost == o.boost)
  end
  alias :== :eql?

  def hash
    return boost.hash
  end
end
