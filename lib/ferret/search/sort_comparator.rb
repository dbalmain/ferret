module Ferret::Search
  # Abstract base class for sorting hits returned by a Query.
  # 
  # This class should only be used if the other SortField types (SCORE, DOC,
  # STRING, INTEGER, FLOAT) do not provide an adequate sorting.  It maintains
  # an internal cache of values which could be quite large.  The cache is an
  # array of Comparable, one for each document in the index.  There is a
  # distinct Comparable for each unique term in the field - if some documents
  # have the same term in the field, the cache array will have entries which
  # reference the same Comparable.
  # 
  # Author::  Tim Jones
  class SortComparator

    # Creates a comparator for the field in the given index.
    #
    # reader:: Index to create comparator for.
    # field_name::  Field to create comparator for.
    # returns:: Comparator of ScoreDoc objects.
    def new_comparator(reader, field_name)
      cached_values = FieldCache::DEFAULT.custom(reader, field, self)
      
      score_doc_comparator =  ScoreDocComparator.new() 

      class <<score_doc_comparator
        attr_writer :cache_values
        def compare(i, j) 
          return @cached_values[i.doc] <=> @cached_values[j.doc]
        end

        def sort_value(i) 
          return @cached_values[i.doc]
        end

        def sort_type()
          return SortField::SortType::CUSTOM
        end
      end
      score_doc_comparator.cached_values = cached_values
      return score_doc_comparator
    end

    # Returns an object which, when sorted according to natural order, will
    # order the Term values in the correct order.  For example, if the Terms
    # contained integer values, this method would return +term_text.to_i+.
    # Note that this might not always be the most efficient implementation -
    # for this particular example, a better implementation might be to make a
    # ScoreDocLookupComparator that uses an internal lookup table of int.
    #
    # term_text:: The textual value of the term.
    #
    # returns:: An object representing +term_text+ that sorts according to the
    #           natural order of +term_text+.
    #
    # See ScoreDocComparator
    def get_comparable(term_text)
      raise NotImplementedError
    end
  end
end
