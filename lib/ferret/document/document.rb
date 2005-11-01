module Ferret::Document
  # Documents are the unit of indexing and search.
  #
  # A Document is a set of fields.  Each field has a name and a textual
  # value.  A field may be Field#stored?() with the document, in which case
  # it is returned with search hits on the document.  Thus each document
  # should typically contain one or more stored fields which uniquely
  # identify it.
  #
  # Note that fields which are _not_ Field#stored?() are _not_ available in
  # documents retrieved from the index, e.g. with Hits#doc, Searcher#doc or
  # IndexReader#document.
  #
  # Several fields may be added with the same name.  In this case, if the
  # fields are indexed, their text is treated as though appended for the
  # purposes of search.
  #
  # Note that add like the remove_field(s) methods only makes sense prior to
  # adding a document to an index. These methods cannot be used to change
  # the content of an existing index! In order to achieve this, a document
  # has to be deleted from an index and a new changed version of that
  # document has to be added.
  class Document
    attr_accessor :boost

    # Constructs a new document with no fields.
    def initialize()
      # Values are multiplied into the value of Field#boost of each field in
      # this document.  Thus, this method in effect sets a default boost for
      # the fields of this document.
      #
      # The default value is 1.0.
      #
      # Note: This value is not stored directly with the document in the
      # index.  Documents returned from IndexReader#document and Hits#doc
      # may thus not have the same value present as when this document was
      # indexed.
      @boost = 1.0
      @fields = {}
    end

    # Returns an array of all fields. Note that it is possible for two
    # fields to appear with the same field name. These will be concatenated
    # in the index.
    def all_fields
      @fields.values.flatten
    end

    # Returns the number of distinct fields held within the document. This
    # counts fields which have multiple entries as one.
    def field_count()
      return @fields.size
    end

    # Returns the number of entries held within the document. This counts
    # all sections so for fields which have multiple entries, each entry
    # is counted
    def entry_count()
      return @fields.values.flatten.size
    end

    # Adds a field to a document.  Several fields may be added with the same
    # name.  In this case, if the fields are indexed, their text is treated
    # as though appended for the purposes of search.
    #
    # Note that add like the remove_field(s) methods only makes sense prior
    # to adding a document to an index. These methods cannot be used to
    # change the content of an existing index! In order to achieve this, a
    # document has to be deleted from an index and a new changed version of
    # that document has to be added.
    def add_field(field)
      (@fields[field.name.to_s] ||= []) << field
    end
    alias :<< :add_field

    # Removes the first field of this name if it exists.
    def remove_field(name)
      @fields[name.to_s].delete_at(0)
    end

    # Removes all fields with the given name from the document.
    #
    # If there is no field with the specified name, the document remains
    # unchanged.
    #
    # Note that the remove_field(s) methods like the add method only make
    # sense prior to adding a document to an index. These methods cannot be
    # used to change the content of an existing index! In order to achieve
    # this, a document has to be deleted from an index and a new changed
    # version of that document has to be added.
    def remove_fields(name)
      @fields.delete(name.to_s)
    end

    # Returns the first field with the given name.
    # This method can return _nil_.
    #
    # name:: the name of the field
    # Return:: a _Field_ array
    def field(name)
      @fields[name.to_s] ? @fields[name.to_s][0] : nil
    end

    # Returns an array of all fields with the given name.
    # This method can return _nil_.
    #
    # name:: the name of the field
    # Return:: a _Field_ array
    def fields(name)
      @fields[name.to_s]
    end

    # Returns an array of values of the field specified as the method
    # parameter.  This method can return _nil_.
    #
    # name:: the name of the field
    # Return:: a _String_ of field values
    def values(name)
      return nil if @fields[name.to_s].nil?
      @fields[name.to_s].map {|f| f.data if not f.binary? }.join(" ")
    end
    alias :[] :values

    # Sets the data in field +field+ to +text+. If there is more than one
    # field of that name then it will set the data in the first field of that
    # name.
    def []=(field_name, data)
      field = field(field_name.to_s)
      raise ArgumentError, "Field does not exist" unless field
      field.data = data
    end

    # Returns an array of binaries of the field specified as the method
    # parameter.  This method can return _nil_.
    #
    # name:: the name of the field
    # Return:: a _String_ of field values
    def binaries(name)
      binaries = []
      @fields[name.to_s].each {|f| binaries << f.data if f.binary? }
      return binaries
    end

    # Prints the fields of a document for human consumption.#/
    def to_s()
      return "Document<#{@fields.keys.join(" ")}>"
    end
  end
end
