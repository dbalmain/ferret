module Ferret::Search
  # Expert: Compares two ScoreDoc objects for sorting.
  class ScoreDocComparator 

    # Special comparator for sorting hits according to computed relevance (score). 
    RELEVANCE = ScoreDocComparator.new() 
    class <<RELEVANCE
      def compare(i, j) 
        return -(i.score <=> j.score)
      end
      def sort_value(i) 
        return i.score
      end
      def sort_type() 
        return SortField::SortType::SCORE
      end
    end


    # Special comparator for sorting hits according to index order (number). 
    INDEX_ORDER = ScoreDocComparator.new() 
    class <<INDEX_ORDER
      def compare(i, j) 
        return i.doc <=> j.doc
      end
      def sort_value(i) 
        return i.doc
      end
      def sort_type() 
        return SortField::SortType::DOC
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

  class SimpleFieldComparator < ScoreDocComparator
    def initialize(index, sort_type)
      @index = index
      @sort_type = sort_type
    end

    def compare(j, i) 
      return @index[i.doc] <=> @index[j.doc]
    end
    def sort_value(i) 
      return @index[i.doc]
    end
    def sort_type() 
      return @sort_type
    end
  end

  class SpecialFieldComparator < SimpleFieldComparator
    def initialize(index, sort_type, comparator)
      super(index, sort_type)
      @comparator = comparator
    end
    def compare(j, i) 
      return @comparator.call(@index[i.doc], @index[j.doc])
    end
  end

  class StringFieldComparator < ScoreDocComparator
    def initialize(index)
      @str_index = index.str_index
      @str_map = index.str_map
    end

    def compare(i, j) 
      return @str_index[i.doc] <=> @str_index[j.doc]
    end
    def sort_value(i) 
      return @str_map[@str_index[i.doc]]
    end
    def sort_type() 
      return SortField::SortType::STRING
    end
  end
end
