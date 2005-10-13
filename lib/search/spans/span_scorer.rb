module Ferret::Search::Spans
  class SpanScorer < Scorer 

    def initialize(spans, weight, similarity, norms)
      @first_time = true
      @more = true
     
      super(similarity)
      @spans = spans
      @norms = norms
      @weight = weight
      @value = weight.value()
      @freq = 0.0
    end

    def next?
      if (@first_time) 
        @more = @spans.next?
        @first_time = false
      end

      return false if not @more

      @freq = 0.0
      @doc = @spans.doc

      while (@more and @doc == @spans.doc) 
        match_length = @spans.finish - @spans.start
        @freq += similarity().sloppy_freq(match_length)
        @more = @spans.next?
      end

      return (@more or @freq != 0.0)
    end

    def doc() return @doc end

    def score()
      raw = similarity().tf(@freq) * @value           # raw score
      return raw * Similarity.decode_norm(@norms[@doc]) # normalize
    end

    def skip_to(target)
      @more = @spans.skip_to(target)

      return false if not @more

      @freq = 0.0
      @doc = @spans.doc()

      while (@more and @spans.doc() == target) 
        @freq += similarity().sloppy_freq(@spans.finish - @spans.start)
        @more = @spans.next?
      end

      return (@more or @freq != 0.0)
    end

    def explain(doc)
      tf_explanation = Explanation.new()

      skip_to(doc)

      phrase_freq = ((doc() == doc) ? @freq : 0.0)
      tf_explanation.value = similarity().tf(phrase_freq)
      tf_explanation.description = "tf(phrase_freq=#{phrase_freq})"

      return tf_explanation
    end

  end
end
