module Ferret::Search
  # Encapsulates sort criteria for returned hits.
  # 
  # The fields used to determine sort order must be carefully chosen.
  # Documents must contain a single term in such a field, and the value of the
  # term should indicate the document's relative position in a given sort
  # order.  The field must be indexed, but should not be tokenized, and does
  # not need to be stored (unless you happen to want it back with the rest of
  # your document data).  In other words:
  # 
  #   document << Field.new("by_number",
  #                         x.to_s,
  #                         Field::Store::NO,
  #                         Field::Index::UN_TOKENIZED))
  # 
  # 
  # === Valid Types of Values
  # 
  # There are three possible kinds of term values which may be put into
  # sorting fields: Integers, Floats, or Strings.  Unless SortField objects
  # are specified, the type of value in the field is determined by parsing the
  # first term in the field.
  # 
  # Integer term values should contain only digits and an optional preceeding
  # negative sign.  Values must be base 10.  Documents which should appear
  # first in the sort should have low value integers, later documents high
  # values (i.e. the documents should be numbered +1..n+ where +1+ is the
  # first and +n+ the last).
  # 
  # Float term values should conform to values accepted by String#to_f.
  # Documents which should appear first in the sort should have low values,
  # later documents high values.
  # 
  # String term values can contain any valid String, but should not be
  # tokenized.  The values are sorted according to their Comparable natural
  # order.  Note that using this type of term value has higher memory
  # requirements than the other two types.
  # 
  # === Object Reuse
  # 
  # One of these objects can be used multiple times and the sort order changed
  # between usages.
  # 
  # This class is thread safe.
  # 
  # === Memory Usage
  # 
  # Sorting uses caches of term values maintained by the internal HitQueue(s).
  # The cache is static and contains an integer or float array of length
  # +IndexReader#max_doc+ for each field name for which a sort is performed.
  # In other words, the size of the cache in bytes is:
  # 
  #   4 * IndexReader#max_doc * (# of different fields actually used to sort)
  # 
  # For String fields, the cache is larger: in addition to the above array,
  # the value of every term in the field is kept in memory.  If there are many
  # unique terms in the field, this could be quite large.
  # 
  # Note that the size of the cache is not affected by how many fields are in
  # the index and _might_ be used to sort - only by the ones actually used to
  # sort a result set.
  # 
  # The cache is cleared each time a new +IndexReader+ is passed in, or if the
  # value returned by +max_doc()+ changes for the current IndexReader.  This
  # class is not set up to be able to efficiently sort hits from more than one
  # index simultaneously.
  class Sort

    attr_accessor :fields

    # Sorts by computed relevance. This is the same sort criteria as calling
    # Searcher#search(Query) Searcher#search()without a sort criteria,
    # only with slightly more overhead.
    def initialize(fields = [SortField::FIELD_SCORE, SortField::FIELD_DOC],
                   reverse = false)
      fields = [fields] unless fields.is_a?(Array)
      @fields = fields
      if fields[0].is_a?(String)
        @fields = fields.map do |field|
          SortField.new(field, SortField::SortBy::AUTO, reverse)
        end
        @fields << SortField::FIELD_DOC if @fields.size == 1
      end
    end

    # Represents sorting by computed relevance. Using this sort criteria returns
    # the same results as calling
    # Searcher#search(Query) Searcher#search()without a sort criteria,
    # only with slightly more overhead.
    RELEVANCE = Sort.new()

    # Represents sorting by index order. 
    INDEX_ORDER = Sort.new(SortField::FIELD_DOC)

    def to_s() 
      return @fields.map {|field| "#{field}"}.join(", ")
    end
  end
end
