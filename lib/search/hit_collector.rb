module Ferret::Search
  # Lower-level search API.
  #
  # HitCollectors are primarily meant to be used to implement queries, sorting
  # and filtering.
  #
  # See Searcher#search(Query, HitCollector)
  class HitCollector 
    # Called once for every non-zero scoring document, with the document number
    # and its score.
    # 
    # If, for example, an application wished to collect all of the hits for a
    # query in a BitSet, then it might:
    #
    #   searcher = IndexSearcher.new(index_reader)
    #   bits = BitSet.new(index_reader.max_doc())
    #   searcher.search(query, HitCollector.new() 
    #     def collect(doc, score) 
    #       bits.set(doc)
    #     end
    #   end
    # 
    # NOTE: This is called in an inner search loop.  For good search
    # performance, implementations of this method should not call
    # Searcher#doc(int) or IndexReader#document(int) on every document number
    # encountered.  Doing so can slow searches by an order of magnitude or more.
    #
    # NOTE: The +score+ passed to this method is a raw score.  In other words,
    # the score will not necessarily be a float whose value is between 0 and 1.
    def collect(doc, score)
      raise NotImplementedError
    end
  end
end
