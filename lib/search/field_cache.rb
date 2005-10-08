require 'monitor'
# Expert: The default cache implementation, storing all values in memory.
# A WeakHashMap is used for storage.
class FieldCacheImpl
  include MonitorMixin

  # Expert: Every key in the internal cache is of this type. 
  class Entry 
    # Creates one of these objects. 
    def initialize(field, type, comparator = nil) 
      if (comparator and type != SortField::SortBy::CUSTOM)
        raise ArgumentError,
          "Only a custom sort field can be created with a custom comparator."
      end

      @field = field
      @type = type
      @comparator = comparator
    end

    # Creates one of these objects for a custom comparator. 
    def Entry.new_custom(field, comparator) 
      return Entry.new(field, SortField::SortBy::CUSTOM, comparator)
    end

    # Two of these are equal iff they reference the same field and type. 
    def eql?(o) 
      return (o.instance_of? Entry and o.field = @field and
              o.type = @type and o.comparator = @comparator)
    end
    alias :== :eql?

    # Composes a hashcode based on the field and type. 
    def hash() 
      return @field.hash ^ @type.hash ^ (@comparator==nil ? 0 : @comparator.hash)
    end
  end

  INT_PARSER = lambda {|i| i.to_i}

  FLOAT_PARSER = lambda {|i| i.to_f}

  # The internal cache. Maps Entry to array of interpreted term values.
  @@cache = {} # should make this a weak hash map

  # See if an object is in the cache. 
  def lookup(reader, field, type, comparator = nil) 
    entry = Entry.new(field, type)
    synchronized(this) 
      reader_cache = (HashMap)cache.get(reader)
      return nil if reader_cache.nil?
      return reader_cache[entry]
    end
  end

  # See if a custom object is in the cache. 
  def lookup_custom(reader, field, comparator) 
    lookup(reader, field, SortField::SortBy::CUSTOM, comparator)
  end

  # Put an object into the cache. 
  Object store (IndexReader reader, String field, int type, Object value) 
    Entry entry = new Entry (field, type)
    synchronized (this) 
      HashMap reader_cache = (HashMap)cache.get(reader)
      if (reader_cache == nil) 
        reader_cache = HashMap.new()
        cache.put(reader,reader_cache)
      end
      return reader_cache.put (entry, value)
    end
  end

  # Put a custom object into the cache. 
  Object store (IndexReader reader, String field, Object comparer, Object value) 
    Entry entry = new Entry (field, comparer)
    synchronized (this) 
      HashMap reader_cache = (HashMap)cache.get(reader)
      if (reader_cache == nil) 
        reader_cache = HashMap.new()
        cache.put(reader, reader_cache)
      end
      return reader_cache.put (entry, value)
    end
  end

  # inherit javadocs
  def getInts (reader, field)
    return getInts(reader, field, INT_PARSER)
  end

  # inherit javadocs
  def getInts (reader, field, parser)
 
    field = field.intern()
    Object ret = lookup (reader, field, parser)
    if (ret == nil) 
      def retArray = new int[reader.maxDoc()]
      if (retArray.length > 0) 
        TermDocs termDocs = reader.termDocs()
        TermEnum termEnum = reader.terms (new Term (field, ""))
        begin 
          if (termEnum.term() == nil) 
            raise new RuntimeException ("no terms in field " + field)
          end
          do 
            Term term = termEnum.term()
            if (term.field() != field) break
             termval = Integer.parseInt (term.text())
            termDocs.seek (termEnum)
            while (termDocs.next()) 
              retArray[termDocs.doc()] = termval
            end
          endwhile (termEnum.next())
        ensure 
          termDocs.close()
          termEnum.close()
        end
      end
      store (reader, field, parser, retArray)
      return retArray
    end
    return (int[]) ret
  end

  # inherit javadocs
  public float[] getFloats (reader, field)
   
    return getFloats(reader, field, FLOAT_PARSER)
  end

  # inherit javadocs
  public float[] getFloats (IndexReader reader, String field,
                            FloatParser parser)
    field = field.intern()
    Object ret = lookup (reader, field, parser)
    if (ret == nil) 
      final float[] retArray = new float[reader.maxDoc()]
      if (retArray.length > 0) 
        TermDocs termDocs = reader.termDocs()
        TermEnum termEnum = reader.terms (new Term (field, ""))
        begin 
          if (termEnum.term() == nil) 
            raise new RuntimeException ("no terms in field " + field)
          end
          do 
            Term term = termEnum.term()
            if (term.field() != field) break
            float termval = Float.parseFloat (term.text())
            termDocs.seek (termEnum)
            while (termDocs.next()) 
              retArray[termDocs.doc()] = termval
            end
          endwhile (termEnum.next())
        ensure 
          termDocs.close()
          termEnum.close()
        end
      end
      store (reader, field, parser, retArray)
      return retArray
    end
    return (float[]) ret
  end

  # inherit javadocs
  def getStrings (reader, field)
 
    field = field.intern()
    Object ret = lookup (reader, field, SortField.STRING)
    if (ret == nil) 
      def retArray = new String[reader.maxDoc()]
      if (retArray.length > 0) 
        TermDocs termDocs = reader.termDocs()
        TermEnum termEnum = reader.terms (new Term (field, ""))
        begin 
          if (termEnum.term() == nil) 
            raise new RuntimeException ("no terms in field " + field)
          end
          do 
            Term term = termEnum.term()
            if (term.field() != field) break
             termval = term.text()
            termDocs.seek (termEnum)
            while (termDocs.next()) 
              retArray[termDocs.doc()] = termval
            end
          endwhile (termEnum.next())
        ensure 
          termDocs.close()
          termEnum.close()
        end
      end
      store (reader, field, SortField.STRING, retArray)
      return retArray
    end
    return (String[]) ret
  end

  # inherit javadocs
  defIndex getStringIndex (reader, field)
 
    field = field.intern()
    Object ret = lookup (reader, field, STRING_INDEX)
    if (ret == nil) 
      def retArray = new int[reader.maxDoc()]
      def mterms = new String[reader.maxDoc()+1]
      if (retArray.length > 0) 
        TermDocs termDocs = reader.termDocs()
        TermEnum termEnum = reader.terms (new Term (field, ""))
        def t = 0;  # current term number

        # an entry for documents that have no terms in this field
        # should a document with no terms be at top or bottom?
        # this puts them at the top - if it is changed, FieldDocSortedHitQueue
        # needs to change as well.
        mterms[t += 1] = nil

        begin 
          if (termEnum.term() == nil) 
            raise new RuntimeException ("no terms in field " + field)
          end
          do 
            Term term = termEnum.term()
            if (term.field() != field) break

            # store term text
            # we expect that there is at most one term per document
            if (t >= mterms.length) raise new RuntimeException ("there are more terms than " +
            		"documents in field \"" + field + "\", but it's impossible to sort on " +
            		"tokenized fields")
            mterms[t] = term.text()

            termDocs.seek (termEnum)
            while (termDocs.next()) 
              retArray[termDocs.doc()] = t
            end

            t += 1
          endwhile (termEnum.next())
        ensure 
          termDocs.close()
          termEnum.close()
        end

        if (t == 0) 
          # if there are no terms, make the term array
          # have a single nil entry
          mterms = new String[1]
        elsif (t < mterms.length) 
          # if there are less terms than documents,
          # trim off the dead array space
          def terms = new String[t]
          System.arraycopy (mterms, 0, terms, 0, t)
          mterms = terms
        end
      end
      defIndex value = new StringIndex (retArray, mterms)
      store (reader, field, STRING_INDEX, value)
      return value
    end
    return (StringIndex) ret
  end

  # The pattern used to detect integer values in a field 
  # removed for java 1.3 compatibility
   protected static final Pattern pIntegers = Pattern.compile ("[0-9\\-]+")
  # 

  # The pattern used to detect float values in a field 
  # removed for java 1.3 compatibility
  # protected static final Object pFloats = Pattern.compile ("[0-9+\\-\\.eEfFdD]+")

  # inherit javadocs
  public Object getAuto (reader, field)
 
    field = field.intern()
    Object ret = lookup (reader, field, SortField.AUTO)
    if (ret == nil) 
      TermEnum enumerator = reader.terms (new Term (field, ""))
      begin 
        Term term = enumerator.term()
        if (term == nil) 
          raise new RuntimeException ("no terms in field " + field + " - cannot determine sort type")
        end
        if (term.field() == field) 
           termtext = term.text().trim()

          # Java 1.4 level code:

           if (pIntegers.matcher(termtext).matches())
           return IntegerSortedHitQueue.comparator (reader, enumerator, field)

           elsif (pFloats.matcher(termtext).matches())
           return FloatSortedHitQueue.comparator (reader, enumerator, field)

          # Java 1.3 level code:
          begin 
            Integer.parseInt (termtext)
            ret = getInts (reader, field)
          rescue (nfe1) 
            begin 
              Float.parseFloat (termtext)
              ret = getFloats (reader, field)
            rescue (nfe2) 
              ret = getStringIndex (reader, field)
            end
          end
          if (ret != nil) 
            store (reader, field, SortField.AUTO, ret)
          end
        else 
          raise new RuntimeException ("field \"" + field + "\" does not appear to be indexed")
        end
      ensure 
        enumerator.close()
      end

    end
    return ret
  end

  # inherit javadocs
  public Comparable[] getCustom (reader, field, comparator)
 
    field = field.intern()
    Object ret = lookup (reader, field, comparator)
    if (ret == nil) 
      final Comparable[] retArray = new Comparable[reader.maxDoc()]
      if (retArray.length > 0) 
        TermDocs termDocs = reader.termDocs()
        TermEnum termEnum = reader.terms (new Term (field, ""))
        begin 
          if (termEnum.term() == nil) 
            raise new RuntimeException ("no terms in field " + field)
          end
          do 
            Term term = termEnum.term()
            if (term.field() != field) break
            Comparable termval = comparator.getComparable (term.text())
            termDocs.seek (termEnum)
            while (termDocs.next()) 
              retArray[termDocs.doc()] = termval
            end
          endwhile (termEnum.next())
        ensure 
          termDocs.close()
          termEnum.close()
        end
      end
      store (reader, field, comparator, retArray)
      return retArray
    end
    return (Comparable[]) ret
  end

end

