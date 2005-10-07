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
    def initialize(directory)
      @reader = IndexReader.open(directory)
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

    def search(query, filter = nil, num_docs = 10)
      if (num_docs <= 0)  # nil might be returned from hq.top() below.
        raise ArgumentError, "num_docs must be > 0 to run a search"
      end

      scorer = query.weight(self).scorer(@reader)
      if (scorer == nil)
        return TopDocs.new(0, [])
      end

      bits = (filter.nil? ? nil : filter.bits(@reader))
      hq = HitQueue.new(num_docs)
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

    # inherit javadoc
#      def search(query, filter, num_docs, sort)
#       
#        scorer = query.weight(self).scorer(@reader)
#        if (scorer == nil)
#          return TopFieldDocs.new(0, [], sort.fields)
#
#        bits = filter != nil ? filter.bits(@reader) : nil
#        hq = FieldSortedHitQueue.new(@reader, sort.fields, num_docs)
#        total_hits = [0]
#        scorer.score() do |doc, score|
#          if score > 0.0 and (bits == nil or bits.get(doc)) # skip docs not in bits
#            total_hits[0] += 1
#            hq.insert(FieldDoc.new(doc, score))
#          end
#        end
#
#        ScoreDoc[] score_docs = new ScoreDoc[hq.size()]
#        for (int i = hq.size()-1; i >= 0; i -= 1)        # put docs in array
#          score_docs[i] = hq.fillFields ((FieldDoc) hq.pop())
#
#        return TopFieldDocs.new(total_hits[0], score_docs, hq.getFields())
#      end


    # inherit javadoc
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
