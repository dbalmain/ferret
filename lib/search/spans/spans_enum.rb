module Ferret::Search::Spans
  # Expert: an enumeration of span matches.  Used to implement span searching.
  # Each span represents a range of term positions within a document.  Matches
  # are enumerated in order, by increasing document number, within that by
  # increasing start position and ensure by increasing finish position. 
  class SpansEnum 
    # Move to the next match, returning true iff any such exists. 
    def next?()
      raise NotImplementedError
    end

    # Skips to the first match beyond the current, whose document number is
    # greater than or equal to _target_. Returns true iff there is such a
    # match.  Behaves as if written:
    #
    #    def skip_to(target) 
    #      begin 
    #        return false if (!next?)
    #      end while (target > doc)
    #      return true
    # end
    #
    # Most implementations are considerably more efficient than that.
    def skip_to(target)
      raise NotImplementedError
    end

    # Returns the document number of the current match.  Initially invalid. 
    def doc()
      raise NotImplementedError
    end


    # Returns the start position of the current match.  Initially invalid. 
    def start()
      raise NotImplementedError
    end

    # Returns the finish position of the current match.  Initially invalid. 
    def finish()
      raise NotImplementedError
    end
  end
end
