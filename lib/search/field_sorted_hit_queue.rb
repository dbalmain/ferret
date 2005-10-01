require 'monitor'

module Ferret::Search
  # Expert: A hit queue for sorting by hits by terms in more than one field.
  # Uses +FieldCache::DEFAULT+ for maintaining internal term lookup tables.
  class FieldSortedHitQueue < PriorityQueue 
    # Stores a comparator corresponding to each field being sorted by 
    attr_accessor :comparators

    # Stores the sort criteria being used. 
    attr_accessor :fields

    # Creates a hit queue sorted by the given list of fields.
    #
    # reader::  Index to use.
    # fields:: Field names, in priority order (highest priority first).
    #          Cannot be +nil+ or empty.  size::  The number of hits to
    #          retain.  Must be greater than zero.
    # raises:: IOError
    def initialize(reader, fields, size)
      super(size)
      n = fields.length
      @comparators = Array.new(n)
      @fields = Array.new(n)
      fields.each_with_index do |field, i|
        @comparators[i] = get_cached_comparator(reader,
                                                field.name,
                                                field.type,
                                                field.locale,
                                                field.comparator)
        @fields[i] = SortField.new(field.name,
                                   comparators[i].sort_type,
                                   field.reverse?)
      end

      # Stores the maximum score value encountered, for normalizing.
      # we only care about scores greater than 1.0 - if all the scores
      # are less than 1.0, we don't have to normalize. 
      @max_score = 1.0
    end


    # Returns whether +a+ is less relevant than +b+.
    # a:: ScoreDoc
    # b:: ScoreDoc
    # returns:: +true+ if document +a+ should be sorted after document +b+.
    def less_than(score_doc1, score_doc2) 
      # keep track of maximum score
      if (score_doc1.score > @max_score) @max_score = score_doc1.score
      if (score_doc2.score > @max_score) @max_score = score_doc2.score

      # run comparators
      c = 0

      @comparators.length.times do |i|
        if @fields[i].reverse?
          c = @comparators[i].compare(score_doc2, score_doc1)
        else
          c = @comparators[i].compare(score_doc1, score_doc2)
        end
        break unless c == 0
      end

      # avoid random sort order that could lead to duplicates
      if (c == 0)
        return score_doc1.doc > score_doc2.doc
      return c > 0
    end


    # Given a FieldDoc object, stores the values used
    # to sort the given document.  These values are not the raw
    # values out of the index, but the internal representation
    # of them.  This is so the given search hit can be collated
    # by a MultiSearcher with other search hits.
    # doc:: The FieldDoc to store sort values into.
    # returns::  The same FieldDoc passed in.
    # See Searchable#search(Weight,Filter,int,Sort)
    def fill_fields(doc) 
      fields = Array.new(@comparators.length)
      @comparators.each do |comparator|
        fields[i] = comparator.sort_value(doc)
      end
      doc.fields = fields
      if (@max_score > 1.0)
        doc.score /= @max_score # normalize scores
      end
      return doc
    end


    # Internal cache of comparators. Similar to FieldCache, only
    # caches comparators instead of term values. 
    @@comparators = Hash.new.extend(MonitorMixin)

    # Returns a comparator if it is in the cache. 
    def FieldSortedHitQueue.lookup(reader, field, type, factory) 
      if factory
        entry = FieldCacheImpl::Entry.new(field, factory)
      else 
        entry = FieldCacheImpl::Entry.new(field, type)
      end
      @@comparators.synchronize do
        reader_cache = @@comparators[reader]
        return nil if reader_cache.nil?
        return reader_cache[entry]
      end
    end

    # Stores a comparator into the cache. 
    def FieldSortedHitQueue.store(reader, field, type, factory, value) 
      if factory
        entry = FieldCacheImpl::Entry.new(field, factory)
      else 
        entry = FieldCacheImpl::Entry.new(field, type)
      end
      @@comparators.synchronize do 
        reader_cache = @@comparators[reader]
        if reader_cache.nil?
          reader_cache = Hash.new()
          @@comparators[reader] = reader_cache
        end
        return reader_cache[entry] = value
      end
    end

    def FieldSortedHitQueue.get_cached_comparator(reader, fieldname,
                                                  type, locale, factory)
      return ScoreDocComparator::INDEXORDER if type == SortField::DOC
      return ScoreDocComparator::RELEVANCE  if type == SortField::SCORE

      comparator = lookup(reader, fieldname, type, factory)
      if (comparator == nil) 
        case (type) 
        when SortField.AUTO:  comparator = comparator_auto(reader, fieldname)
        when SortField.INT:   comparator = comparator_int(reader, fieldname)
        when SortField.FLOAT: comparator = comparator_float(reader, fieldname)
        when SortField.STRING
          if locale
            comparator = comparator_string_locale(reader, fieldname, locale)
          else
            comparator = comparator_string(reader, fieldname)
          end
        when SortField.CUSTOM: comparator = factory.new_comparator(reader, fieldname)
        else: raise ArgumentError, "unknown field type: " + type
        end
        store (reader, fieldname, type, factory, comparator)
      end
      return comparator
    end

    # Returns a comparator for sorting hits according to a field containing integers.
    # reader::  Index to use.
    # fieldname::  Field containg integer values.
    # returns::  Comparator for sorting hits.
    # raises:: IOException If an error occurs reading the index.
    def comparator_int(reader, fieldname)
      field_order = FieldCache::DEFAULT.get_ints(reader, fieldname)
      score_doc_comparator = ScoreDocComparator.new()
      def score_doc_comparator.sort_type()
        return SortField::INT
      end
      compare_block = Proc.new {|i, j| field_order[i.doc] <=> field_order[j.doc]}
      score_doc_comparator.class.send(:define_method, :compare, compare_block)
      sort_value_block = Proc.new {|i| field_order[i.doc]}
      score_doc_comparator.class.send(:define_method, :sort_value, sort_value_block)

      return score_doc_comparator
    end

    # Returns a comparator for sorting hits according to a field containing floats.
    # reader::  Index to use.
    # fieldname::  Field containg float values.
    # returns::  Comparator for sorting hits.
    # raises:: IOException If an error occurs reading the index.
    def comparator_float(reader, fieldname)
   
      field_order = FieldCache::DEFAULT.get_floats(reader, fieldname)
      score_doc_comparator = ScoreDocComparator.new()
      def score_doc_comparator.sort_type()
        return SortField::FLOAT
      end
      compare_block = Proc.new {|i, j| field_order[i.doc] <=> field_order[j.doc]}
      score_doc_comparator.class.send(:define_method, :compare, compare_block)
      sort_value_block = Proc.new {|score_doc| field_order[score_doc.doc]}
      score_doc_comparator.class.send(:define_method, :sort_value, sort_value_block)

      return score_doc_comparator
    end

    # Returns a comparator for sorting hits according to a field containing strings.
    # reader::  Index to use.
    # fieldname::  Field containg string values.
    # returns::  Comparator for sorting hits.
    # raises:: IOException If an error occurs reading the index.
    static ScoreDocComparator comparator_string (final IndexReader reader, final String fieldname)
   
       field = fieldname.intern()
      final FieldCache.StringIndex index = FieldCache.DEFAULT.getStringIndex (reader, field)
      return new ScoreDocComparator () 

        def compare (final ScoreDoc i, final ScoreDoc j) 
          def fi = index.order[i.doc]
          def fj = index.order[j.doc]
          if (fi < fj) return -1
          if (fi > fj) return 1
          return 0
        end

        public Comparable sort_value (final ScoreDoc i) 
          return index.lookup[index.order[i.doc]]
        end

        def sort_type() 
          return SortField.STRING
        end
      end
    end

    # Returns a comparator for sorting hits according to a field containing strings.
    # reader::  Index to use.
    # fieldname::  Field containg string values.
    # returns::  Comparator for sorting hits.
    # raises:: IOException If an error occurs reading the index.
    static ScoreDocComparator comparator_string_locale (final IndexReader reader, final String fieldname, final Locale locale)
   
      final Collator collator = Collator.getInstance (locale)
       field = fieldname.intern()
      def index = FieldCache.DEFAULT.getStrings (reader, field)
      return ScoreDocComparator.new() 

        def compare (final ScoreDoc i, final ScoreDoc j) 
          return collator.compare (index[i.doc], index[j.doc])
        end

        public Comparable sort_value (final ScoreDoc i) 
          return index[i.doc]
        end

        def sort_type() 
          return SortField.STRING
        end
      end
    end

    # Returns a comparator for sorting hits according to values in the given field.
    # The terms in the field are looked at to determine whether they contain integers,
    # floats or strings.  Once the type is determined, one of the other static methods
    # in this class is called to get the comparator.
    # reader::  Index to use.
    # fieldname::  Field containg values.
    # returns::  Comparator for sorting hits.
    # raises:: IOException If an error occurs reading the index.
    static ScoreDocComparator comparator_auto (final IndexReader reader, final String fieldname)
   
       field = fieldname.intern()
      Object lookupArray = FieldCache.DEFAULT.getAuto (reader, field)
      if (lookupArray instanceof FieldCache.StringIndex) 
        return comparator_string (reader, field)
      elsif (lookupArray instanceof int[]) 
        return comparator_int (reader, field)
      elsif (lookupArray instanceof float[]) 
        return comparator_float (reader, field)
      elsif (lookupArray instanceof String[]) 
        return comparator_string (reader, field)
      else 
        raise new RuntimeException ("unknown data type in field '"+field+"'")
      end
    end
  end
end
