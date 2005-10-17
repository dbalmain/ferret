module Ferret
  module Index
    # Abstract class for enumerating terms.
    #
    # Term enumerations are always ordered by Term.<=>.  Each term in
    # the enumeration is greater than all that precede it.
    class TermEnum
      # Increments the enumeration to the next element.  True if one exists.
      def next?
        raise NotImplementedError
      end

      # Returns the current Term in the enumeration.
      def term
        raise NotImplementedError
      end

      # Returns the doc_freq of the current Term in the enumeration.
      def doc_freq
        raise NotImplementedError
      end

      # Closes the enumeration to further activity, freeing resources.
      def close
        raise NotImplementedError
      end
      
      # Term Vector support
      # Skips terms to the first beyond the current whose value is
      # greater or equal to _target_.
      #
      # Returns true iff there is such a term.
      #
      # Behaves as if written:
      #
      #   def skip_to(target_term)
      #     while (target > term)
      #       if (!next()) return false
      #     end
      #     return true
      #   end
      #
      # Some implementations are considerably more efficient than that.
      def skip_to(term)
        while (target > term)
          return false if not next?
        end
        return true
      end
    end
  end
end
