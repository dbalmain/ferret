module Ferret::Search
  # Expert: Compares two ScoreDoc objects for sorting.
  class ScoreDocComparator 

    # Special comparator for sorting hits according to computed relevance (score). 
    RELEVANCE = ScoreDocComparator.new() 
    class <<RELEVANCE
      def compare (i, j) 
        return -1 if (i.score > j.score)
        return 1 if (i.score < j.score)
        return 0
      end
      def sort_value (i) 
        return i.score
      end
      def sort_type() 
        return SortField::SCORE
      end
    end


    # Special comparator for sorting hits according to index order (number). 
    INDEXORDER = ScoreDocComparator.new() 
    class <<INDEXORDER
      def compare (i, j) 
        return -1 if (i.doc < j.doc)
        return 1  if (i.doc > j.doc)
        return 0
      end
      def sort_value (i) 
        return i.doc
      end
      def sort_type() 
        return SortField::DOC
      end
    end


    # Compares two ScoreDoc objects and returns a result indicating their
    # sort order.
    # i:: First ScoreDoc
    # j:: Second ScoreDoc
    # returns:: +-1+ if +i+ should come before +j+
    #           +1+  if +i+ should come after +j+
    #           +0+  if they are equal
    def compare(i, j)
      return NotImplementedError
    end


    # Returns the value used to sort the given document.  The object returned
    # must implement the java.io.Serializable interface.  This is used by
    # multisearchers to determine how to collate results from their searchers.
    #
    # See FieldDoc
    # i:: Document
    # returns:: Serializable object
    def sort_value(i)
      return NotImplementedError
    end


    # Returns the type of sort.  Should return +SortField.SCORE+,
    # +SortField.DOC+, +SortField.STRING+, +SortField.INTEGER+,
    # +SortField.FLOAT+ or +SortField.CUSTOM+.  It is not valid to return
    # +SortField.AUTO+.
    # This is used by multisearchers to determine how to collate results from
    # their searchers.  returns:: One of the constants in SortField.
    # See SortField
    def sort_type()
      return NotImplementedError
    end
  end
end
