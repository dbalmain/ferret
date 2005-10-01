module Ferret
  module Index
    # TermDocs provides an interface for enumerating &lt;document,
    # frequency&gt; pairs for a term. 
    #
    # The document portion names each document containing the term.  Documents
    # are indicated by number.  The frequency portion gives the number of times
    # the term occurred in each document. 
    # 
    # The pairs are ordered by document number.
    #
    # See IndexReader#term_docs
    class TermDocs
      # Sets this to the data for a term.
      # The enumeration is reset to the start of the data for this term.
      def seek(term) raise NotImplementedError end

      # Returns the current document number.
      #
      # This is invalid until #next() is called for the first time.
      def doc() raise NotImplementedError end

      # Returns the frequency of the term within the current document. This
      # is invalid until {@link #next()} is called for the first time.
      def freq() raise NotImplementedError end

      # Moves to the next pair in the enumeration.
      # Returns true iff there is such a next pair in the enumeration.
      def next?() raise NotImplementedError end

      # Attempts to read multiple entries from the enumeration, up to length of
      # _docs_.  Document numbers are stored in _docs_, and term
      # frequencies are stored in _freqs_.  The _freqs_ array must be as
      # long as the _docs_ array.
      #
      # Returns the number of entries read.  Zero is only returned when the
      # stream has been exhausted.
      def read(docs, freqs)  raise NotImplementedError end

      # Skips entries to the first beyond the current whose document number is
      # greater than or equal to _target_. 
      # 
      # Returns true iff there is such an entry. 
      # 
      # Some implementations are considerably more efficient than that.
      def skip_to(target)
        while (target > doc())
          return false if not next?()
        end
        return true
      end

      # Frees associated resources.
      def close() raise NotImplementedError end
    end
  end
end
