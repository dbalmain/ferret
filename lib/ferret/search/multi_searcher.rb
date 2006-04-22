module Ferret::Search

  # Implements searching multiple IndexSearchers at once
  # 
  # Applications usually need only call the @link #search(Query)
  # or @link #search(Query,Filter) methods. For performance reasons it is 
  # recommended to open only one Searcher and use it for all of your searches.
  class MultiSearcher
    include Ferret::Index

    attr_accessor :similarity, :searchers

    # Creates a MultiSearcher searching across all the searchers
    # in the provided array.
    #
    def initialize(args)
      @searchers = Array.new(args)
      @similarity = Similarity.default
      
      # initialize reader lookup array
      @max_doc = 0
      @starts = Array.new(@searchers.size + 1)
      @searchers.each_with_index { |searcher, i| 
        @starts[i] = @max_doc
        @max_doc += searcher.max_doc
      }
      @starts[@searchers.size] = @max_doc
    end
    
    # closes all underlying Searchers
    def close()
      @searchers.each { |searcher| searcher.close() }
    end

    # Expert: Returns the number of documents containing +term+.
    # Called by search code to compute term weights.
    # See IndexReader#doc_freq
    def doc_freq(term)
      return @searchers.inject(0) { |df, searcher| 
        df + searcher.doc_freq(term) 
      }
    end

    # Expert: For each term in the terms array, calculates the number of
    # documents containing +term+. Returns an array with these
    # document frequencies. Used to minimize number of remote calls.
    def doc_freqs(terms)
      result = Array.new
      terms.each {|term, i| result << doc_freq(term)}
      return result
    end

    # Expert: Returns the stored fields of document +n+.
    #
    # See IndexReader#get_document
    def doc(n)
      i = sub_searcher(n)
      return @searchers[i].doc(n - @starts[i])
    end

    # Returns index of the searcher for document <code>n</code> in the 
    # array used to construct this searcher. 
    def sub_searcher(n)
      lo = 0			            # search starts array
      hi = @searchers.size - 1  # for first element less
						                  # than n, return its index
      while hi >= lo do
        mid = (lo + hi) >> 1
        midValue = @starts[mid]
        if n < midValue
          hi = mid - 1;
        elsif n > midValue
          lo = mid + 1;
        else                   # found a match
          while mid+1 < @searchers.size && @starts[mid+1] == midValue do
            mid += 1                # scan to last match
          end
          return mid
        end
      end
      return hi
    end

    # Returns the document number of document <code>n</code> within its
    # sub-index.
    def sub_doc(n)
      return n - @starts[sub_searcher(n)]
    end
    
    # Expert: Returns one greater than the largest possible document number.
    # Called by search code to compute term weights.
    # See IndexReader#max_doc
    def max_doc
      return @max_doc
    end

    # Create weight in multiple index scenario.
    # 
    # Distributed query processing is done in the following steps:
    # 1. rewrite query
    # 2. extract necessary terms
    # 3. collect dfs for these terms from the Searchables
    # 4. create query weight using aggregate dfs.
    # 5. distribute that weight to Searchables
    # 6. merge results
    #
    # Steps 1-4 are done here, 5+6 in the search() methods
    def create_weight(query)
      # step 1
      rewritten_query = self.rewrite(query)

      # step 2
      terms = Set.new
      rewritten_query.extract_terms(terms)

      # step 3
      aggregated_dfs = Array.new(terms.size, 0)
      @searchers.each { |searcher|
        dfs = searcher.doc_freqs(terms)
        dfs.each_with_index { |df,i|
          aggregated_dfs[i] += df
        }
      }

      df_map = Hash.new
      terms.each_with_index { |term,i|
        df_map[term] = aggregated_dfs[i]
      }

      # step 4
      cache_sim = CachedDfSource.new(df_map, self.max_doc, self.similarity)
      
      return rewritten_query.weight(cache_sim)
    end

   
    def search(query, options = {})
      filter = options[:filter]
      first_doc = options[:first_doc]||0
      num_docs = options[:num_docs]||10
      max_size = first_doc + num_docs
      sort = options[:sort]

      if (num_docs <= 0)
        raise ArgumentError, "num_docs must be > 0 to run a search"
      end

      if (first_doc < 0)
        raise ArgumentError, "first_doc must be >= 0 to run a search"
      end


      if (sort)
        raise NotImplementedError
        #fields = sort.is_a?(Array) ? sort : sort.fields
        #hq = FieldDocSortedHitQueue.new(fields, max_size)
      else
        hq = HitQueue.new(max_size)
      end

      total_hits = 0
      weight = create_weight(query)
      @searchers.each_with_index { |searcher,i|     # search each searcher
        docs = searcher.search(weight, 
                               :filter => filter,
                               #:sort => sort,
                               :num_docs => max_size,
                               :first_doc => 0)
        total_hits += docs.total_hits  # update total_hits
        docs.score_docs.each { |score_doc|
          score_doc.doc += @starts[i]   # convert doc
          break unless hq.insert(score_doc) # no more scores > min_score
        }
      }
      
      score_docs = []
      if (hq.size > first_doc)
        if (hq.size - first_doc) < num_docs 
          num_docs = hq.size - first_doc
        end
        num_docs.times do
          score_docs.unshift(hq.pop)
        end
      end
      hq.clear

      return TopDocs.new(total_hits, score_docs)
    end

    def search_each(query, filter = nil, &block)
      weight = create_weight(query)
      @searchers.each { |searcher|     # search each searcher
        searcher.search_each(weight, filter, &block)
      }
    end

    # rewrites the query into a query that can be processed by the search
    # methods. For example, a Fuzzy query is turned into a massive boolean
    # query.
    #
    # original:: The original query to be rewritten.
    def rewrite(original)
      #print "multi_searcher#rewrite: #{original}\n"
      queries = []
      @searchers.each { |searcher|
        queries << searcher.rewrite(original)
      }
      return queries.first.combine(queries)
    end

    # Returns an Explanation that describes how +doc+ scored against
    # +query+.
    # 
    # This is intended to be used in developing Similarity implementations,
    # and, for good performance, should not be displayed with every hit.
    # Computing an explanation is as expensive as executing the query over the
    # entire index.
    def explain(query, doc)
      i = sub_searcher(doc)
      return @searchers[i].explain(create_weight(query), doc-@starts[i])
    end

  end


  # Document Frequency cache acting as a Dummy-Searcher.
  # This class is no full-fledged Searcher, but only supports
  # the methods necessary to initialize Weights.
  class CachedDfSource

    attr_reader :max_doc, :similarity
    
    def initialize(df_map, max_doc, similarity)
      @df_map = df_map
      @max_doc = max_doc
      @similarity = similarity
    end
      
    def doc_freq(term)
      return @df_map[term]
    end

    def doc_freqs(terms)
      result = Array.new
      terms.each { |term|
        result << doc_freq(term)
      }
      return result
    end

    def rewrite(query)
      # this is a bit of a hack. We know that a query which
      # creates a Weight based on this Dummy-Searcher is
      # always already rewritten (see preparedWeight()).
      # Therefore we just return the unmodified query here
      return query
    end

  end

end
