module Ferret::Search
  require 'monitor'

  # Expert: The default cache implementation, storing all values in memory.
  # A WeakKeyHash is used for storage.
  class FieldCache
    include Ferret::Index

    StringIndex = Struct.new(:str_index, :str_map)

    # Expert: Every key in the internal cache is of this type. 
    class Entry 
      attr_reader :field, :sort_type, :comparator
      # Creates one of these objects. 
      def initialize(field, sort_type, comparator = nil) 
        @field = field
        @sort_type = sort_type
        @comparator = comparator
      end

      # Two of these are equal iff they reference the same field and sort_type. 
      def eql?(o) 
        return (o.instance_of? Entry and o.field == @field and
                o.sort_type == @sort_type and o.comparator == comparator)
      end
      alias :== :eql?

      # Composes a hashcode based on the field and sort_type. 
      def hash() 
        return @field.hash ^ @sort_type.hash ^ @comparator.hash
      end
    end

    INT_PARSER = lambda {|i| i.to_i}

    FLOAT_PARSER = lambda {|i| i.to_f}

    # The internal cache. Maps Entry to array of interpreted term values.
    @@cache = Ferret::Utils::WeakKeyHash.new.extend(MonitorMixin)

    # See if an object is in the cache. 
    def FieldCache.lookup(reader, field, sort_type) 
      entry = Entry.new(field, sort_type)
      @@cache.synchronize() do
        reader_cache = @@cache[reader]
        return nil if reader_cache.nil?
        return reader_cache[entry]
      end
    end

    # Put an object into the cache. 
    def FieldCache.store(reader, field, sort_type, value) 
      entry = Entry.new(field, sort_type)
      @@cache.synchronize() do
        reader_cache = @@cache[reader]
        if (reader_cache == nil) 
          reader_cache = {}
          @@cache[reader] = reader_cache
        end
        return reader_cache[entry] = value
      end
    end

    # Checks the internal cache for an appropriate entry, and if none is found,
    # reads the terms in +field+ and parses them with the provided parser and
    # returns an array of size +reader.max_doc+ of the value each document has
    # in the given field.
    #
    # reader::     Used to get field values.
    # field::      Which field contains the values.
    # sort_type::  The type of sort to run on the field. Holds the parser
    # return::     The values in the given field for each document.
    def FieldCache.get_index(reader, field, sort_type)
      index = lookup(reader, field, sort_type)
      if (index == nil) 
        parser = sort_type.parser
        index = Array.new(reader.max_doc)
        if (index.length > 0) 
          term_docs = reader.term_docs
          term_enum = reader.terms_from(Term.new(field, ""))
          begin 
            if term_enum.term.nil? 
              raise "no terms in field '#{field}' to sort by"
            end
            begin 
              term = term_enum.term
              break if (term.field != field)
              termval = parser.call(term.text)
              term_docs.seek(term_enum)
              while term_docs.next? 
                index[term_docs.doc] = termval
              end
            end while term_enum.next?
          ensure 
            term_docs.close()
            term_enum.close()
          end
        end
        store(reader, field, sort_type, index)
      end
      return index
    end

    # Checks the internal cache for an appropriate entry, and if none is found
    # reads the term values in +field+ and returns an array of them in natural
    # order, along with an array telling which element in the term array each
    # document uses.
    #
    # reader::  Used to get field values.
    # field::   Which field contains the strings.
    # returns:: Array of terms and index into the array for each document.
    def FieldCache.get_string_index(reader, field)
      index = lookup(reader, field, SortField::SortType::STRING)
      if (index == nil) 
        str_index = Array.new(reader.max_doc)
        str_map = Array.new(reader.max_doc+1)
        if (str_index.length > 0) 
          term_docs = reader.term_docs
          term_enum = reader.terms_from(Term.new(field,""))
          t = 0 # current term number

          # an entry for documents that have no terms in this field should a
          # document with no terms be at top or bottom?
          #
          # this puts them at the top - if it is changed, FieldDocSortedHitQueue
          # needs to change as well.
          str_map[t] = nil
          t += 1

          begin 
            if (term_enum.term() == nil) 
              raise "no terms in field #{field} to sort by"
            end
            begin
              term = term_enum.term
              break if (term.field != field)

              # store term text
              # we expect that there is at most one term per document
              if (t >= str_map.length)
                raise "there are more terms than documents in field \"#{field}\", but it's impossible to sort on tokenized fields"
              end
              str_map[t] = term.text

              term_docs.seek(term_enum)
              while term_docs.next?
                str_index[term_docs.doc] = t
              end

              t += 1
            end while term_enum.next?
          ensure 
            term_docs.close()
            term_enum.close()
          end

          if (t == 0) 
            # if there are no terms, make the term array
            # have a single nil entry
            # str_map = [nil] <= already set above
          elsif (t < str_map.length) 
            # if there are less terms than documents,
            # trim off the dead array space
            str_map.compact!
          end
        end
        index = StringIndex.new(str_index, str_map)
        store(reader, field, SortField::SortType::STRING, index)
      end
      return index
    end

    # Checks the internal cache for an appropriate entry, and if none is found
    # reads +field+ to see if it contains integers, floats or strings, and then
    # calls one of the other methods in this class to get the values.  For
    # string values, a StringIndex is returned.  After calling this method,
    # there is an entry in the cache for both type +AUTO+ and the actual found
    # type.
    #
    # reader:: Used to get field values.
    # field::  Which field contains the values.
    # return:: Integer Array, Float Array or StringIndex.
    def FieldCache.get_auto_index(reader, field)
      index = lookup(reader, field, SortField::SortType::AUTO)
      if (index == nil) 
        term_enum = reader.terms_from(Term.new(field, ""))
        begin 
          term = term_enum.term
          if (term == nil) 
            raise "no terms in field #{field} to sort by"
          end
          if (term.field == field) 
            termtext = term.text.strip

            if (termtext == termtext.to_i.to_s)
              index = get_index(reader, field, SortField::SortType::INT)
            elsif (termtext == termtext.to_f.to_s or termtext == "%f"%termtext.to_f)
              index = get_index(reader, field, SortField::SortType::FLOAT)
            else
              index = get_string_index(reader, field)
            end

            if (index != nil) 
              store(reader, field, SortField::SortType::AUTO, index)
            end
          else 
            raise "field \"#{field}\" does not appear to be indexed"
          end
        ensure 
          term_enum.close()
        end
      end
      return index
    end
  end
end
