# Stores information about how to sort documents by terms in an individual
# field.  Fields must be indexed in order to sort by them.
class SortField
  # Sort by document score (relevancy).  Sort values are Float and higher
  # values are at the front. 
  SCORE = 0

  # Sort by document number (order).  Sort values are Integer and lower
  # values are at the front. 
  DOC = 1

  # Guess type of sort based on field contents.  A regular expression is used
  # to look at the first term indexed for the field and determine if it
  # represents an integer number, a floating point number, or just arbitrary
  # string characters. 
  AUTO = 2

  # Sort using term values as Strings.  Sort values are String and lower
  # values are at the front. 
  STRING = 3

  # Sort using term values as encoded Integers.  Sort values are Integer and
  # lower values are at the front. 
  INT = 4

  # Sort using term values as encoded Floats.  Sort values are Float and
  # lower values are at the front. 
  FLOAT = 5

  # Sort using a custom Comparator.  Sort values are any Comparable and
  # sorting is done according to natural order. 
  CUSTOM = 9

  # IMPLEMENTATION NOTE: the FieldCache::STRING_INDEX is in the same "namespace"
  # as the above static int values.  Any new values must not have the same value
  # as FieldCache::STRING_INDEX.

  attr_reader :name, :type, :comparator, :locale

  def reverse?
    return @reverse
  end

  # Creates a sort by terms in the given field where the type of term value
  # is determined dynamically (#AUTO AUTO).
  # name:: Name of field to sort by.  Can be +nil+ if +type+ is SCORE or DOC.
  # type:: Type of values in the terms.
  # reverse:: True if natural order should be reversed.
  # locale:: Locale of values in the field.
  # comparator:: A comparator for sorting hits.
  def initialize(name = nil,
                 type = AUTO,
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
  FIELD_SCORE = SortField.new(nil, SCORE)

  # Represents sorting by document number (order). 
  FIELD_DOC = SortField.new(nil, DOC)


  def to_s() 
    buffer = ""
    case @type 
    when SCORE: buffer << "<score>"
    when DOC: buffer << "<doc>"
    when CUSTOM: buffer << "<custom:\"" + name + "\": "
    else buffer << "\"" + @name + "\""
    end

    buffer << "("+locale+")" if @locale
    buffer << '!' if @reverse

    return buffer
  end
end
