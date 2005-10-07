include Ferret::Utils

module Ferret::Search
  # Stores information about how to sort documents by terms in an individual
  # field.  Fields must be indexed in order to sort by them.
  class SortField
    class SortBy < Parameter
      # Sort by document score (relevancy).  Sort values are Float and higher
      # values are at the front. 
      SCORE = SortBy.new("score")

      # Sort by document number (order).  Sort values are Integer and lower
      # values are at the front. 
      DOC = SortBy.new("doc")

      # Guess type of sort based on field contents.  A regular expression is
      # used to look at the first term indexed for the field and determine if
      # it represents an integer number, a floating point number, or just
      # arbitrary string characters. 
      AUTO = SortBy.new("auto")

      # Sort using term values as Strings.  Sort values are String and lower
      # values are at the front. 
      STRING = SortBy.new("string")

      # Sort using term values as encoded Integers.  Sort values are Integer
      # and lower values are at the front. 
      INT = SortBy.new("int")

      # Sort using term values as encoded Floats.  Sort values are Float and
      # lower values are at the front. 
      FLOAT = SortBy.new("float")

      # Sort using a custom Comparator.  Sort values are any Comparable and
      # sorting is done according to natural order. 
      CUSTOM = SortBy.new("custom")
    end


    attr_reader :name, :type, :comparator, :locale

    def reverse?
      return @reverse
    end

    # Creates a sort by terms in the given field where the type of term value
    # is determined dynamically (#AUTO AUTO).
    #
    # name:: Name of field to sort by.  Can be +nil+ if +type+ is SCORE or
    #     DOC.
    # type:: Type of values in the terms.
    # reverse:: True if natural order should be reversed.
    # comparator:: A comparator for sorting hits.
    # locale:: Locale of values in the field.
    def initialize(name = nil,
                   type = SortBy::AUTO,
                   reverse = false,
                   comparator = nil,
                   locale = nil)
      @name = name
      @type = type             # defaults to determining type dynamically
      @reverse = reverse       # defaults to natural order
      @comparator = comparator
      @locale = locale         # defaults to no locale
    end

    # Represents sorting by document score (relevancy). 
    FIELD_SCORE = SortField.new(nil, SortBy::SCORE)

    # Represents sorting by document number (order). 
    FIELD_DOC = SortField.new(nil, SortBy::DOC)


    def to_s() 
      buffer = '"' + (@name||"<#{@type}>") + '"'
      buffer << "("+locale+")" if @locale
      buffer << '!' if @reverse

      return buffer
    end
  end
end
