# The interface for search implementations.
# 
# Searchable is the abstract network protocol for searching. 
# Implementations provide search over a single index, over multiple
# indices, and over indices on remote servers.
# 
# Queries, filters and sort criteria are designed to be compact so that
# they may be efficiently passed to a remote index, with only the top-scoring
# hits being returned, rather than every non-zero scoring hit.
class Searchable
  # Lower-level search API.
  # 
  # HitCollector#collect(int,float) is called for every non-zero
  # scoring document.
  # 
  # HitCollector-based access to remote indexes is discouraged.
  # 
  # Applications should only use this if they need _all_ of the
  # matching documents.  The high-level search API (@link
  # Searcher#search(Query)end) is usually more efficient, as it skips
  # non-high-scoring hits.
  # 
  # weight:: to match documents
  # filter:: if non-nil, a bitset used to eliminate some documents
  # results:: to receive hits
  # raises:: BooleanQuery.TooManyClauses
  def search(weight, filter, results)
 

  # Expert: Low-level search implementation.
  # @deprecated use Searcher#search(Query, Filter, HitCollector) instead.
  def search(query, filter, results)
   

  # Frees resources associated with this Searcher.
  # Be careful not to call this method while you are still using objects
  # like Hits.
  def close()

  # Expert: Returns the number of documents containing +term+.
  # Called by search code to compute term weights.
  # @see IndexReader#doc_freq(Term)
  def doc_freq(term)

  # Expert: For each term in the terms array, calculates the number of
  # documents containing +term+. Returns an array with these
  # document frequencies. Used to minimize number of remote calls.
  def doc_freqs(terms)

  # Expert: Returns one greater than the largest possible document number.
  # Called by search code to compute term weights.
  # @see IndexReader#max_doc()
  def max_doc()

  # Expert: Low-level search implementation.  Finds the top +n+
  # hits for +query+, applying +filter+ if non-nil.
  # 
  # Called by Hits.
  # 
  # Applications should usually call Searcher#search(Query) or
  # Searcher#search(Query,Filter) instead.
  # raises:: BooleanQuery.TooManyClauses
  TopDocs search(weight, filter, n)

  # Expert: Low-level search implementation.
  # @deprecated use Searcher#search(Query, Filter, int) instead.
  TopDocs search(query, filter, n)

  # Expert: Returns the stored fields of document +i+.
  # Called by HitCollector implementations.
  # @see IndexReader#document(int)
  Document doc(i)

  # Expert: called to re-write queries into primitive queries.
  # raises:: BooleanQuery.TooManyClauses
  Query rewrite(query)

  # Expert: low-level implementation method
  # Returns an Explanation that describes how +doc+ scored against
  # +weight+.
  # 
  # This is intended to be used in developing Similarity implementations,
  # and, for good performance, should not be displayed with every hit.
  # Computing an explanation is as expensive as executing the query over the
  # entire index.
  # Applications should call Searcher#explain(Query, int).
  # raises:: BooleanQuery.TooManyClauses
  Explanation explain(weight, doc)

  # @deprecated use Searcher#explain(Query, int) instead.
  Explanation explain(query, doc)

  # Expert: Low-level search implementation with arbitrary sorting.  Finds
  # the top +n+ hits for +query+, applying
  # +filter+ if non-nil, and sorting the hits by the criteria in
  # +sort+.
  # 
  # Applications should usually call @link
  # Searcher#search(Query,Filter,Sort)endinstead.
  # raises:: BooleanQuery.TooManyClauses
  TopFieldDocs search(Weight weight, Filter filter, int n, Sort sort)
 

  # Expert: Low-level search implementation.
  # @deprecated use Searcher#search(Query, Filter, int, Sort) instead.
  TopFieldDocs search(Query query, Filter filter, int n, Sort sort)
   
end
