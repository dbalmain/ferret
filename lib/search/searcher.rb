# An abstract base class for search implementations.
# Implements some common utility methods.
class Searcher

  # Returns documents matching +query+, filtered by +filter+ and
  # sorted by +sort+.
  # raises:: BooleanQuery.TooManyClauses
  def search(query, filter, sort)
    return Hits.new(this, query, filter, sort)
  end

  # Expert: Low-level search implementation with arbitrary sorting.  Finds the
  # top +n+ hits for +query+, applying +filter+ if non-nil, and sorting the
  # hits by the criteria in +sort+.
  # 
  # Applications should usually call Searcher#search(Query,Filter,Sort)
  # instead.
  #
  # raises:: BooleanQuery::TooManyClauses
  public TopFieldDocs search(Query query, Filter filter, int n,
                                      Sort sort)
    return search(createWeight(query), filter, n, sort)
  end

  # Lower-level search API.
  # 
  # HitCollector#collect(int,float) is called for every non-zero
  # scoring document.
  # 
  # Applications should only use this if they need _all_ of the
  # matching documents.  The high-level search API (@link
  # Searcher#search(Query)end) is usually more efficient, as it skips
  # non-high-scoring hits.
  # Note: The +score+ passed to this method is a raw score.
  # In other words, the score will not necessarily be a float whose value is
  # between 0 and 1.
  # raises:: BooleanQuery.TooManyClauses
  def search(query, results)
   
    search(query, (Filter)nil, results)
  end

  # Lower-level search API.
  # 
  # HitCollector#collect(int,float) is called for every non-zero
  # scoring document.
  # <br>HitCollector-based access to remote indexes is discouraged.
  # 
  # Applications should only use this if they need _all_ of the
  # matching documents.  The high-level search API (@link
  # Searcher#search(Query)end) is usually more efficient, as it skips
  # non-high-scoring hits.
  # 
  # query:: to match documents
  # filter:: if non-nil, a bitset used to eliminate some documents
  # results:: to receive hits
  # raises:: BooleanQuery.TooManyClauses
  def search(query, filter, results)
   
    search(createWeight(query), filter, results)
  end

  # Expert: Low-level search implementation.  Finds the top +n+
  # hits for +query+, applying +filter+ if non-nil.
  # 
  # Called by Hits.
  # 
  # Applications should usually call Searcher#search(Query) or
  # Searcher#search(Query,Filter) instead.
  # raises:: BooleanQuery.TooManyClauses
  public TopDocs search(query, filter, n)
   
    return search(createWeight(query), filter, n)
  end

  # Returns an Explanation that describes how +doc+ scored against
  # +query+.
  # 
  # This is intended to be used in developing Similarity implementations,
  # and, for good performance, should not be displayed with every hit.
  # Computing an explanation is as expensive as executing the query over the
  # entire index.
  public Explanation explain(query, doc)
    return explain(createWeight(query), doc)
  end

  # The Similarity implementation used by this searcher. 
  private Similarity similarity = Similarity.getDefault()

  # Expert: Set the Similarity implementation used by this Searcher.
  # 
  # @see Similarity#setDefault(Similarity)
  def setSimilarity(similarity) 
    @similarity = similarity
  end

  # Expert: Return the Similarity implementation used by this Searcher.
  # 
  # This defaults to the current value of Similarity#getDefault().
  public Similarity getSimilarity() 
    return this.similarity
  end

  # creates a weight for +query+
  # returns:: new weight
  protected Weight createWeight(query)
      return query.weight(this)
  end

  def docFreqs(terms)
    def result = new int[terms.length]
    for (int i = 0; i < terms.length; i += 1) 
      result[i] = docFreq(terms[i])
    end
    return result
  end
end
