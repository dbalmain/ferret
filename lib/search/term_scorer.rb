module Ferret::Search
  # Expert: A +Scorer+ for documents matching a +Term+.
  class TermScorer < Scorer 
    SCORE_CACHE_SIZE = 32
    MAX_DOCS = 0x7FFFFFFF

    # Returns the current document number matching the query.
    # Initially invalid, until #next() is called the first time.
    attr_reader :doc

    # Construct a +TermScorer+.
    # weight:: The weight of the +Term+ in the query.
    # td:: An iterator over the documents matching the +Term+.
    # similarity:: The +Similarity+ implementation to be used for score computations.
    # norms:: The field norms of the document fields for the +Term+.
    def initialize(weight, td, similarity, norms) 
      super(similarity)

      @doc = 0
      @docs = Array.new(32, 0) # buffered doc numbers
      @freqs = Array.new(32, 0) # buffered term freqs
      @pointer = @pointer_max = 0;
      @score_cache = Array.new(SCORE_CACHE_SIZE)

      @weight = weight
      @term_docs = td
      @norms = norms
      @weight_value = weight.value

      SCORE_CACHE_SIZE.times do |i|
        @score_cache[i] = similarity().tf(i) * @weight_value
      end
    end

    def each_hit
      next?()
      each_hit_up_to(MAX_DOCS)
    end

    def each_hit_up_to(last = MAX_DOCS)
      sim = similarity() # cache sim in local
      while (@doc < last) # for docs in window
        f = @freqs[@pointer]

        # compute tf(f)*weight
        if f < SCORE_CACHE_SIZE                    # check cache
          score = @score_cache[f]                  # cache hit
        else
          score = similarity.tf(f) * @weight_value # cache miss
        end

        score *= sim.decode_norm(@norms[@doc])      # normalize for field

        yield(@doc, score)                         # collect score

        @pointer += 1
        if @pointer >= @pointer_max
          @pointer_max = @term_docs.read(@docs, @freqs)  # refill buffers
          if (@pointer_max != 0) 
            @pointer = 0
          else 
            @term_docs.close() # close stream
            @doc = MAX_DOCS    # set to sentinel value
            return false
          end
        end
        @doc = @docs[@pointer]
      end
      return true
    end


    # Advances to the next document matching the query.
    # <br>The iterator over the matching documents is buffered using
    # TermDocEnum#read(int[],int[]).
    # returns:: true iff there is another document matching the query.
    def next?()
      @pointer += 1
      if @pointer >= @pointer_max
        @pointer_max = @term_docs.read(@docs, @freqs) # refill buffer
        if @pointer_max != 0
          @pointer = 0
        else 
          @term_docs.close()                          # close stream
          @doc = MAX_DOCS                             # set to sentinel value
          return false
        end
      end
      @doc = @docs[@pointer]
      return true
    end

    def score() 
      f = @freqs[@pointer]
      # compute tf(f)*weight
      if  f < SCORE_CACHE_SIZE                 # check cache
        raw = @score_cache[f]                  # cache hit
      else
        raw = similarity().tf(f) * @weight_value # cache miss
      end

      return raw * Similarity.decode_norm(norms[@doc]) # normalize for field
    end

    # Skips to the first match beyond the current whose document number is
    # greater than or equal to a given target. 
    # 
    # The implementation uses TermDocEnum#skip_to(int).
    # target:: The target document number.
    # returns:: true iff there is such a match.
    def skip_to(target)
      # first scan in cache
      while (@pointer += 1) < @pointer_max
        if @docs[@pointer] >= target
          @doc = @docs[@pointer]
          return true
        end
      end

      # not found in cache, seek underlying stream
      result = @term_docs.skip_to(target)
      if (result) 
        @pointer_max = 1
        @pointer = 0
        @docs[@pointer] = @doc = @term_docs.doc
        @freqs[@pointer] = @term_docs.freq
      else 
        @doc = Integer.MAX_VALUE
      end
      return result
    end

    # Returns an explanation of the score for a document.
    # 
    # When this method is used, the #next() method and the #score() method
    # should not be used.
    #
    # doc:: The document number for the explanation.
    # TODO: Modify to make use of TermDocEnum#skip_to(int).
    def explain(doc)
      query = @weight.query()
      tf_explanation = Explanation.new()
      tf = 0
      while (@pointer < @pointer_max) 
        if (@docs[@pointer] == doc)
          tf = @freqs[@pointer]
        end
        @pointer += 1
      end
      if (tf == 0) 
        while (@term_docs.next?) 
          if (@term_docs.doc() == doc) 
            tf = @term_docs.freq()
          end
        end
      end
      @term_docs.close()
      tf_explanation.value = similarity().tf(tf)
      tf_explanation.description = "tf(term_freq(#{query.term})=#{tf})"
      
      return tf_explanation
    end

    # Returns a string representation of this +TermScorer+. 
    def to_s() return "scorer(" + @weight + ")"; end
  end
end
