include Ferret::Utils
module Ferret
  module Document
    # A field is a section of a Document.  Each field has two parts, a name
    # and a value.  Values may be free text, provided as a String or as a
    # Reader, or they may be atomic keywords, which are not further processed.
    # Such keywords may be used to represent dates, urls, etc.  Fields are
    # optionally stored in the index, so that they may be returned with hits
    # on the document.
    class Field

      # This value will be
      # multiplied into the score of all hits on this field of this
      # document.
      #
      # The boost is multiplied by Document#boost of the document
      # containing this field.  If a document has multiple fields with the same
      # name, all such values are multiplied together.  This product is then
      # multipled by the value Similarity#length_norm(String,int), and
      # rounded by Similarity#encode_norm(float) before it is stored in the
      # index.  One should attempt to ensure that this product does not overflow
      # the range of that encoding.
      #
      # See Document#set_boost(float)
      # See Similarity#length_norm(String, int)
      # See Similarity#encode_norm(float)
      #
      # Note: this value is not stored directly with the document in the index.
      # Documents returned from IndexReader#document(int) and
      # Hits#doc(int) may thus not have the same value present as when this field
      # was indexed.
      attr_accessor :boost

      attr_reader :name, :data

      # True iff the value of the field is to be stored in the index for
      # return with search hits.  It is an error for this to be true if a
      # field is Reader-valued.
      def stored?() return @stored end

      # True iff the value of the field is to be indexed, so that it may be
      # searched on.
      def indexed?() return @indexed end

      # True iff the value of the field should be tokenized as text prior to
      # indexing.  Un-tokenized fields are indexed as a single word and may
      # not be Reader-valued.
      def tokenized?() return @tokenized end

      # True if the field is to be stored as a binary value. This can be used
      # to store images or other binary data in the index if you wish
      def binary?() return @binary end

      # True if you want to compress the data that you store. This is a good
      # idea for really large text fields. The ruby Zlib library is used to do
      # the compression
      def compressed?() return @compressed end

      # True iff the term or terms used to index this field are stored as a
      # term vector, available from IndexReader#term_freq_vector(). These
      # methods do not provide access to the original content of the field,
      # only to terms used to index it. If the original content must be
      # preserved, use the _stored_ attribute instead.
      #
      # See IndexReader#term_freq_vector()
      def store_term_vector?() return @store_term_vector end

      # True if the positions of the indexed terms in this field are stored.
      def store_position?() return @store_position end

      # True if the offsets of this field are stored. The offsets are the
      # positions of the start and end characters of the token in the whole
      # field string
      def store_offset?() return @store_offset end

      class Store < Parameter
        # Store the original field value in the index in a compressed form.
        # This is useful for long documents and for binary valued fields.
        COMPRESS = Store.new("COMPRESS")

        # Store the original field value in the index. This is useful for
        # short texts like a document's title which should be displayed with
        # the results. The value is stored in its original form, i.e. no
        # analyzer is used before it is stored. 
        YES = Store.new("YES")

        # Do not store the field value in the index.
        NO = Store.new("NO")
      end

      class Index < Parameter
        # Do not index the field value. This field can thus not be searched,
        # but one can still access its contents provided it is Field.Store
        # stored
        NO = Index.new("NO")

        # Index the field's value so it can be searched. An Analyzer will be
        # used to tokenize and possibly further normalize the text before its
        # terms will be stored in the index. This is useful for common text.
        TOKENIZED = Index.new("TOKENIZED")

        # Index the field's value without using an Analyzer, so it can be
        # searched.  As no analyzer is used the value will be stored as a
        # single term. This is useful for unique Ids like product numbers.
        UNTOKENIZED = Index.new("UNTOKENIZED")
      end

      class TermVector < Parameter
        # Do not store term vectors. 
        NO = TermVector.new("NO")

        # Store the term vectors of each document. A term vector is a list of
        # the document's terms and their number of occurences in that
        # document.
        YES = TermVector.new("YES")

        # Store the term vector + token position information
        # 
        # See #YES
        WITH_POSITIONS = TermVector.new("WITH_POSITIONS")

        # Store the term vector + Token offset information
        # 
        # See #YES
        WITH_OFFSETS = TermVector.new("WITH_OFFSETS")

        # Store the term vector + Token position and offset information
        # 
        # See #YES See #WITH_POSITIONS See #WITH_OFFSETS
        WITH_POSITIONS_OFFSETS = TermVector.new("WITH_POSITIONS_OFFSETS")
      end

      # Create a field by specifying its name, value and how it will
      # be saved in the index.
      # 
      # name:: The name of the field
      # value:: The string to process
      # store:: Whether _value_ should be stored in the index
      # index:: Whether the field should be indexed, and if so, if it should
      #         be tokenized before indexing 
      #
      # store_term_vector:: Whether term vector should be stored
      #  * the field is neither stored nor indexed
      #  * the field is not indexed but term_vector is _TermVector::YES_
      def initialize(name,
                     value,
                     stored = Store::YES,
                     index = Index::UNTOKENIZED,
                     store_term_vector = TermVector::NO,
                     binary = false,
                     boost = 1.0)
        if (index == Index::NO and stored == Store::NO)
          raise ArgumentError, "it doesn't make sense to have a field that " +
            "is neither indexed nor stored"
        end
        if (index == Index::NO && store_term_vector != TermVector::NO)
          raise ArgumentError, "cannot store term vector information for a " +
            "field that is not indexed"
        end

        # The name of the field (e.g., "date", "subject", "title", or "body")
        @name = name

        # the one and only data object for all different kind of field values
        @data = value
        self.stored = stored
        self.index = index
        self.store_term_vector = store_term_vector
        @binary = binary
        @boost = boost
      end

      def stored=(stored)
        if (stored == Store::YES)
          @stored = true
          @compressed = false
        elsif (stored == Store::COMPRESS)
          @stored = true
          @compressed = true
        elsif (stored == Store::NO)
          @stored = false
          @compressed = false
        else
          raise "unknown stored parameter " + store.to_s
        end
      end

      def index=(index)
        if (index == Index::NO)
          @indexed = false
          @tokenized = false
        elsif (index == Index::TOKENIZED)
          @indexed = true
          @tokenized = true
        elsif (index == Index::UNTOKENIZED)
          @indexed = true
          @tokenized = false
        else
          raise "unknown stored parameter " + index.to_s
        end
      end

      def store_term_vector=(store_term_vector)
        if (store_term_vector == TermVector::NO)
          @store_term_vector = false
          @store_position = false
          @store_offset = false
        elsif (store_term_vector == TermVector::YES)
          @store_term_vector = true
          @store_position = false
          @store_offset = false
        elsif (store_term_vector == TermVector::WITH_POSITIONS)
          @store_term_vector = true
          @store_position = true
          @store_offset = false
        elsif (store_term_vector == TermVector::WITH_OFFSETS)
          @store_term_vector = true
          @store_position = false
          @store_offset = true
        elsif (store_term_vector == TermVector::WITH_POSITIONS_OFFSETS)
          @store_term_vector = true
          @store_position = true
          @store_offset = true
        else
          raise "unknown term_vector parameter " + store_term_vector.to_s
        end
      end

      # Returns the string value of the data that is stored in this field
      def string_value
        if @data.instance_of? String
          return @data
        elsif @data.respond_to? :read
          return @data.read()
        else
          # if it is binary object try to return a string representation
          return @data.to_s
        end
      end

      # if the data is stored as a binary, just return it.
      def binary_value
        return @data
      end

      # Returns the string value of the data that is stored in this field
      def reader_value
        if @data.respond_to? :read
          return @data
        elsif @data.instance_of? String
          return StringHelper::StringReader.new(@data)
        else
          # if it is binary object try to return a string representation
          return StringHelper::StringReader.new(@data.to_s)
        end
      end

      # Create a stored field with binary value. Optionally the value
      # may be compressed. But it obviously won't be tokenized or
      # term vectored or anything like that.
      # 
      # name:: The name of the field
      # value:: The binary value
      # store:: How _value_ should be stored (compressed or not.)
      def Field.new_binary_field(name, value, stored)
        if (stored == Store::NO)
          raise ArgumentError, "binary values can't be unstored"
        end
        Field.new(name, value, stored, Index::NO, TermVector::NO, true)
      end

      # Prints a Field for human consumption.
      def to_s()
        str = ""
        if (@stored)
          str << "stored"
          @str << @compressed ? "/compressed," : "/uncompressed,"
        end
        if (@indexed) then str << "indexed," end
        if (@tokenized) then str << "tokenized," end
        if (@store_term_vector) then str << "store_term_vector," end
        if (@store_offset)
          str << "term_vector_offsets,"
        end
        if (@store_position)
          str << "term_vector_position,"
        end 
        if (@binary) then str << "binary," end

        str << '<'
        str << @name
        str << ':'

        if (@data != null)
          str << @data.to_s
        end

        str << '>'
      end  
    end
  end
end
