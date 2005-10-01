module Ferret::Search
  # A scorer that matches no document at all. 
  class NonMatchingScorer < Scorer 
    def initialize()
      super(nil) # no similarity used
    end
    
   def next?
     return false
   end

   def skip_to(target)
     return false
   end

   def explain(doc) 
      e = Explanation.new()
      e.description = "No document matches."
      return e
    end
  end
end
