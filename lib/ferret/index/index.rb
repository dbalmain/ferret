module Ferret::Index
  # This is a simplified interface to the index. See the TUTORIAL for more
  # information on how to use this class.
  class Index
    include Ferret::Store
    include Ferret::Search
    include Ferret::Document

    # If you create an Index without any options, it'll simply create an index
    # in memory. But this class is highly configurable and every option that
    # you can supply to IndexWriter and QueryParser, you can also set here.
    #
    # === Options
    #
    # path::              A string representing the path to the index
    #                     directory. If you are creating the index for the
    #                     first time the directory will be created if it's
    #                     missing. You should not choose a directory which
    #                     contains other files.
    # create_if_missing:: Create the index if no index is found in the
    #                     specified directory. Otherwise, use the existing
    #                     index. This defaults to true and has no effect on in
    #                     memory indexes.
    # create::            Creates the index, even if one already exists. That
    #                     means any existing index will be deleted. This
    #                     option defaults to false and has no effect for in
    #                     memory indexes. It is probably better to use the
    #                     create_if_missing option.
    # default_field::     This specifies the field or fields that will be 
    #                     searched by the query parser. You can use a string
    #                     to specify one field, eg, "title". Or you can
    #                     specify multiple fields with a String - 
    #                     "title|content" - or with an Array - ["title",
    #                     "content"]. This defaults to "*" which signifies all
    #                     fields in the index.
    # analyzer::          Sets the default analyzer for the index. This is
    #                     used by both the IndexWriter and the QueryParser to
    #                     tokenize the input. The default is the
    #                     StandardAnalyzer.
    # dir::               This is an Ferret::Store::Directory object. This can
    #                     be useful if you have an already existing in-memory
    #                     index which you'd like to read with this class. If
    #                     you want to create a new index, you are better off
    #                     passing in a path.
    # close_dir::         This specifies whether you would this class to close
    #                     the index directory when this class is closed. This
    #                     only has any meaning when you pass in a directory
    #                     object in the *dir* option, in which case it
    #                     defaults to false. Otherwise it is always true.
    # occur_default::     Set to either BooleanClause::Occur::SHOULD (default)
    #                     or BooleanClause::Occur::MUST to specify the default
    #                     Occur operator.
    # wild_lower::        Set to false if you don't want the terms in fuzzy and
    #                     wild queries to be set to lower case. You should do
    #                     this if your analyzer doesn't downcase. The default
    #                     is true.
    # default_slop::      Set the default slop for phrase queries. This
    #                     defaults to 0.
    # 
    # Some examples;
    #
    #   index = Index::Index.new(:analyzer => WhiteSpaceAnalyzer.new())
    #
    #   index = Index::Index.new(:path => '/path/to/index',
    #                            :create_if_missing => false)
    #
    #   index = Index::Index.new(:dir => directory,
    #                            :close_dir => false
    #                            :default_slop => 2)
    #   
    def initialize(options = {})
      if options[:path]
        options[:create_if_missing] = true if options[:create_if_missing].nil? 
        @dir = FSDirectory.new(options[:path], true)
        options[:close_dir] = true
      elsif options[:dir]
        @dir = options[:dir]
      else
        options[:create] = true # this should always be true for a new RAMDir
        @dir = RAMDirectory.new
      end

      @options = options
      @writer = IndexWriter.new(@dir, options)
      options[:analyzer] = @analyzer = @writer.analyzer
      @has_writes = false
      @reader = nil
      @options.delete(:create) # only want to create the first time if at all
      @close_dir = @options.delete(:close_dir) || false # we'll hold this here
      @default_field = @options[:default_field] || "*"
      @open = true
    end

    # Closes this index by closing its associated reader and writer objects.
    def close
      if not @open
        raise "tried to close an already closed directory"
      end
      @reader.close() if @reader
      @writer.close() if @writer
      @dir.close()

      @open = false
    end

    # Get the reader for this index.
    # NOTE:: This will close the writer from this index.
    def reader
      ensure_reader_open()
      return @reader
    end

    # Get the searcher for this index.
    # NOTE:: This will close the writer from this index.
    def searcher
      ensure_searcher_open()
      return @searcher
    end

    # Get the writer for this index.
    # NOTE:: This will close the reader from this index.
    def writer
      ensure_writer_open()
      return @writer
    end

    # Adds a document to this index, using the provided analyzer instead of
    # the local analyzer if provided.  If the document contains more than
    # IndexWriter::MAX_FIELD_LENGTH terms for a given field, the remainder are
    # discarded.
    def add_document(doc, analyzer = nil)
      ensure_writer_open()
      fdoc = nil
      if doc.is_a?(String)
        fdoc = Document.new
        fdoc << Field.new(@default_field, doc,
                          Field::Store::YES, Field::Index::TOKENIZED)
      elsif doc.is_a?(Array)
        fdoc = Document.new
        doc.each() do |field|
          fdoc << Field.new(@default_field, field,
                            Field::Store::YES, Field::Index::TOKENIZED)
        end
      elsif doc.is_a?(Hash)
        fdoc = Document.new
        doc.each_pair() do |field, text|
          fdoc << Field.new(field.to_s, text.to_s,
                            Field::Store::YES, Field::Index::TOKENIZED)
        end
      elsif doc.is_a?(Document)
        fdoc = doc
      else
        raise ArgumentError, "Unknown document type #{doc.class}"
      end
      @has_writes = true

      @writer.add_document(fdoc, analyzer || @writer.analyzer)
    end
    alias :<< :add_document

    # The main search method for the index. You need to create a query to
    # pass to this method. You can also pass a hash with one or more of the
    # following; {filter, num_docs, first_doc, sort}
    #
    # query::    the query to run on the index
    # filter::   filters docs from the search result
    # first_doc:: The index in the results of the first doc retrieved.
    #   Default is 0
    # num_docs:: The number of results returned. Default is 10
    # sort::     an array of SortFields describing how to sort the results.
    def search(query, options = {})
      ensure_searcher_open()
      if query.is_a?(String)
        if @qp.nil?
          @qp = Ferret::QueryParser.new(@default_field, @options)
        end
        # we need to set this ever time, in case a new field has been added
        @qp.fields = @reader.get_field_names
        query = @qp.parse(query)
      end

      return @searcher.search(query, options)
    end

    # See Index#search
    #
    # This method yields the doc and score for each hit.
    # eg.
    #   index.search_each() do |doc, score|
    #     puts "hit document number #{doc} with a score of #{score}"
    #   end
    #
    def search_each(query, options = {}) # :yield: doc, score
      search(query, options).score_docs.each do |score_doc|
        yield score_doc.doc, score_doc.score
      end
    end

    # Retrieve the document referenced by the document number +id+, if id is
    # an integer or the first document with term +id+ if +id+ is a term.
    #
    # id:: The number of the document to retrieve, or the term used as the id
    #      for the document we wish to retrieve
    def doc(id)
      ensure_reader_open()
      if id.is_a?(String)
        t = Term.new("id", id.to_s)
        return @reader.get_document_with_term(t)
      elsif id.is_a?(Term)
        return @reader.get_document_with_term(id)
      else
        return @reader.get_document(id)
      end
    end
    alias :[] :doc
    
    # Delete the document referenced by the document number +id+ if +id+ is an
    # integer or all of the documents which have the term +id+ if +id+ is a
    # term..
    #
    # id:: The number of the document to delete
    def delete(id)
      ensure_reader_open()
      if id.is_a?(String)
        t = Term.new("id", id.to_s)
        return @reader.delete_docs_with_term(t)
      elsif id.is_a?(Term)
        return @reader.delete_docs_with_term(id)
      else
        return @reader.delete(id)
      end
    end

    # Returns true if document +n+ has been deleted 
    def deleted?(n)
      ensure_reader_open()
      return @reader.deleted?(n) 
    end

    # Returns true if any documents have been deleted since the index was last
    # flushed.
    def has_deletions?()
      ensure_reader_open()
      return @reader.has_deletions?
    end
    
    # Returns true if any documents have been added to the index since the
    # last flush.
    def has_writes?()
      return @has_writes
    end

    # optimizes the index. This should only be called when the index will no
    # longer be updated very often, but will be read a lot.
    def optimize()
      ensure_writer_open()
      @writer.optimize()
      @modified = true
    end

    # returns the number of documents in the index
    def size()
      ensure_reader_open()
      return @reader.num_docs()
    end

    protected
      def ensure_writer_open()
        raise "tried to use a closed index" if not @open
        return if @writer
        if @reader
          @reader.close
          @reader = nil
          @searcher = nil
        end
        @writer = IndexWriter.new(@dir, @options)
      end

      def ensure_reader_open()
        raise "tried to use a closed index" if not @open
        return if @reader
        if @writer
          @writer.close
          @writer = nil
        end
        @reader = IndexReader.open(@dir, false)
      end

      def ensure_searcher_open()
        raise "tried to use a closed index" if not @open
        return if @searcher
        ensure_reader_open()
        @searcher = IndexSearcher.new(@reader)
      end
  end
end
