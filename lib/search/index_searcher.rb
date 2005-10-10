include Ferret::Index
module Ferret::Search
  # Implements search over a single IndexReader.
  # 
  # Applications usually need only call the inherited @link #search(Query)end
  # or @link #search(Query,Filter)endmethods. For performance reasons it is 
  # recommended to open only one IndexSearcher and use it for all of your searches.
  class IndexSearcher
    attr_accessor :similarity
    attr_reader :reader

    # Creates a searcher searching the index in the provided directory. 
    def initialize(arg)
      if arg.is_a?(IndexReader)
        @reader = arg
      elsif arg.is_a?(Directory)
        @reader = IndexReader.open(arg)
      elsif arg.is_a?(String)
        @dir = FSDirectory.open(arg)
        @reader = IndexReader.open(@dir, true)
      else
        raise ArgumentError, "Unknown argument passed to initialize IndexReader"
      end

      @similarity = Similarity.default
    end
    
    # IndexSearcher was constructed with IndexSearcher(r).
    # If the IndexReader was supplied implicitly by specifying a directory, then
    # the IndexReader gets closed.
    def close()
      @reader.close()
    end

    def doc_freq(term)
      return @reader.doc_freq(term)
    end

    def doc_freqs(terms)
      result = Array.new(terms.length)
      terms.each_with_index {|term, i| result[i] = doc_freq(term)}
      return result
    end

    def doc(i)
      return @reader.document(i)
    end

    def max_doc()
      return @reader.max_doc()
    end

    def create_weight(query)
      return query.weight(self)
    end

    # The main search method for the index. You need to create a query to
    # pass to this method. You can also pass a hash with one or more of the
    # following; {filter, num_docs, sort}
    #
    # query::    the query to run on the index
    # filter::   filters docs from the search result
    # num_docs:: the number of docs to return. The default is 10.
    # sort::     an array of SortFields describing how to sort the results.
    def search(query, args = {})
      filter = args[:filter]
      num_docs = args[:num_docs]||10
      sort = args[:sort]

      if (num_docs <= 0)  # nil might be returned from hq.top() below.
        raise ArgumentError, "num_docs must be > 0 to run a search"
      end

      scorer = query.weight(self).scorer(@reader)
      if (scorer == nil)
        return TopDocs.new(0, [])
      end

      bits = (filter.nil? ? nil : filter.bits(@reader))
      if (sort)
        fields = sort.is_a?(Array) ? sort : sort.fields
        hq = FieldSortedHitQueue.new(@reader, fields, num_docs)
      else
        hq = HitQueue.new(num_docs)
      end
      total_hits = 0
      min_score = 0.0
      scorer.each_hit() do |doc, score|
        if score > 0.0 and (bits.nil? or bits.get(doc)) # skip docs not in bits
          total_hits += 1
          if hq.size < num_docs or score >= min_score 
            hq.insert(ScoreDoc.new(doc, score))
            min_score = hq.top.score # maintain min_score
          end
        end
      end

      score_docs = Array.new(hq.size)
      (hq.size - 1).downto(0) do |i|
        score_docs[i] = hq.pop
      end

      return TopDocs.new(total_hits, score_docs)
    end

    def search_collect(query, filter = nil)
      scorer = query.weight(self).scorer(@reader)
      return if scorer == nil
      bits = (filter.nil? ? nil : filter.bits(@reader))
      scorer.each_hit() do |doc, score|
        if score > 0.0 and (bits.nil? or bits.get(doc)) # skip docs not in bits
          yield(doc, score)
        end
      end
    end

#      def search(query, filter, results)
#        HitCollector collector = results
#        if (filter != nil) 
#          final BitSet bits = filter.bits(@reader)
#          collector = HitCollector.new() 
#              def collect(doc, score) 
#                if (bits.get(doc)) # skip docs not in bits
#                  results.collect(doc, score)
#                end
#              end
#            end
#        end
#
#        Scorer scorer = query.weight(this).scorer(@reader)
#        if (scorer == nil)
#          return
#        scorer.score(collector)
#      end

    def rewrite(original)
      query = original
      rewritten_query = query.rewrite(@reader)
      while query != rewritten_query
        query = rewritten_query
        rewritten_query = query.rewrite(@reader)
      end
      return query
    end

    def explain(query, doc)
      return query.weight(self).explain(@reader, doc)
    end
  end
end
