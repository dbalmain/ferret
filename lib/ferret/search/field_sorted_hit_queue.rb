require 'monitor'

module Ferret::Search
  # Expert: A hit queue for sorting by hits by terms in more than one field.
  # Uses +FieldCache+ for maintaining internal term lookup tables.
  class FieldSortedHitQueue < Ferret::Utils::PriorityQueue 
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
        @comparators[i] = get_cached_comparator(reader, field)
        @fields[i] = SortField.new(field.name,
                                   {:sort_type => comparators[i].sort_type,
                                    :reverse => field.reverse?})
      end

      # Stores the maximum score value encountered, for normalizing.
      # we only care about scores greater than 1.0 - if all the scores
      # are less than 1.0, we don't have to normalize. 
      @max_score = 1.0
    end


    # Returns whether +a+ is less relevant than +b+.
    # sd1:: ScoreDoc
    # sd2:: ScoreDoc
    # returns:: +true+ if document +a+ should be sorted after document +b+.
    def less_than(sd1, sd2) 
      # keep track of maximum score
      @max_score = sd1.score if (sd1.score > @max_score)
      @max_score = sd2.score if (sd2.score > @max_score)

      # run comparators
      c = 0

      @comparators.length.times do |i|
        if @fields[i].reverse?
          c = @comparators[i].compare(sd2, sd1)
        else
          c = @comparators[i].compare(sd1, sd2)
        end
        break unless c == 0
      end

      # avoid random sort order that could lead to duplicates
      if (c == 0)
        return sd1.doc > sd2.doc
      end
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
    end

    # Internal cache of comparators. Similar to FieldCache, only
    # caches comparators instead of term values. 
    @@comparators = Ferret::Utils::WeakKeyHash.new.extend(MonitorMixin)

    # Returns a comparator if it is in the cache. 
    def lookup(reader, field, sort_type, comproc) 
      entry = FieldCache::Entry.new(field, sort_type, comproc)
      @@comparators.synchronize() do
        reader_cache = @@comparators[reader]
        return nil if reader_cache.nil?
        return reader_cache[entry]
      end
    end

    # Stores a comparator into the cache. 
    def store(reader, field, sort_type, comproc, value) 
      entry = FieldCache::Entry.new(field, sort_type, comproc)
      @@comparators.synchronize do 
        reader_cache = @@comparators[reader]
        if reader_cache.nil?
          reader_cache = Hash.new()
          @@comparators[reader] = reader_cache
        end
        return reader_cache[entry] = value
      end
    end

    def get_cached_comparator(reader, field)
      if field.sort_type == SortField::SortType::DOC
        return ScoreDocComparator::INDEX_ORDER
      end
      if field.sort_type == SortField::SortType::SCORE
        return ScoreDocComparator::RELEVANCE 
      end

      comparator = lookup(reader, field.name, field.sort_type, field.comparator)
      if (comparator == nil) 
        case (field.sort_type) 
        when SortField::SortType::AUTO: 
          comparator = comparator_auto(reader, field.name)
        when SortField::SortType::STRING: 
          comparator = comparator_string(reader, field.name)
        else 
          comparator = comparator_simple(reader, field)
        end

        store(reader, field.name, field.sort_type, field.comparator, comparator)
      end
      return comparator
    end

    # Returns a comparator for sorting hits according to the sort type and the
    # comparator function passed.
    # strings. 
    #
    # reader::  Index to use.
    # field::   Lets us know which field to search and how to parse it.
    # returns:: Comparator for sorting hits.
    def comparator_simple(reader, field)
      index = FieldCache.get_index(reader, field.name, field.sort_type)
      comproc = field.comparator
      if (comproc)
        return SpecialFieldComparator.new(index, field.sort_type, comproc)
      else
        return SimpleFieldComparator.new(index, field.sort_type)
      end
    end

    # Returns a comparator for sorting hits according to a field containing
    # strings. 
    #
    # reader::  Index to use.
    # field::   Field containing string values.
    # returns:: Comparator for sorting hits.
    def comparator_string(reader, field)
      index = FieldCache.get_string_index(reader, field)
      return StringFieldComparator.new(index)
    end

    # Returns a comparator for sorting hits according to values in the given field.
    # The terms in the field are looked at to determine whether they contain integers,
    # floats or strings.  Once the type is determined, one of the other static methods
    # in this class is called to get the comparator.
    # reader::  Index to use.
    # field::  Field containg values.
    # returns::  Comparator for sorting hits.
    # raises:: IOException If an error occurs reading the index.
    def comparator_auto(reader, field)
      index = FieldCache.get_auto_index(reader, field)
      if (index.is_a?(FieldCache::StringIndex))
        return StringFieldComparator.new(index)
      elsif (index[0].is_a?(Integer)) 
        return SimpleFieldComparator.new(index, SortField::SortType::INT)
      elsif (index[0].is_a?(Float)) 
        return SimpleFieldComparator.new(index, SortField::SortType::FLOAT)
      else 
        raise "unknown data type in field '#{field}'. Data = #{index[0]}"
      end
    end
  end
end
