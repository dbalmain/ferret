module Ferret::Search
  # Expert: A ScoreDoc which also contains information about
  # how to sort the referenced document.  In addition to the
  # document number and score, this object contains an array
  # of values for the document from the field(s) used to sort.
  # For example, if the sort criteria was to sort by fields
  # "a", "b" then "c", the +fields+ object array
  # will have three elements, corresponding respectively to
  # the term values for the document in fields "a", "b" and "c".
  # The class of each element in the array will be either
  # Integer, Float or String depending on the type of values
  # in the terms of each field.
  # 
  class FieldDoc < ScoreDoc 

    # Expert: The values which are used to sort the referenced document.
    # The order of these will match the original sort criteria given by a
    # Sort object.  Each Object will be either an Integer, Float or String,
    # depending on the type of values in the terms of the original field.
    # See Sort
    # See Searcher#search(Query,Filter,int,Sort)
    attr_accessor :fields

    # Expert: Creates one of these objects with the given sort information. 
    def initialize(doc, score, fields = nil)
      super(doc, score)
      @fields = fields
    end

  end
end
