require 'monitor'

module Ferret::Index
  # This is a simplified interface to the index. See the TUTORIAL for more
  # information on how to use this class.
  class Index
    include MonitorMixin

    include Ferret::Store
    include Ferret::Search
    include Ferret::Document

    # If you create an Index without any options, it'll simply create an index
    # in memory. But this class is highly configurable and every option that
    # you can supply to IndexWriter and QueryParser, you can also set here.
    #
    # === Options
    #
    # path::                 A string representing the path to the index
    #                        directory. If you are creating the index for the
    #                        first time the directory will be created if it's
    #                        missing. You should not choose a directory which
    #                        contains other files.
    # create_if_missing::    Create the index if no index is found in the
    #                        specified directory. Otherwise, use the existing
    #                        index. This defaults to true and has no effect on
    #                        in memory indexes.
    # create::               Creates the index, even if one already exists.
    #                        That means any existing index will be deleted.
    #                        This option defaults to false and has no effect
    #                        for in memory indexes. It is probably better to
    #                        use the create_if_missing option.
    # default_field::        This specifies the default field that will be
    #                        used when you add a simple string to the index
    #                        using #add_document. This will also be used for
    #                        default_search_field unless you set it
    #                        explicitly. The default for this value is the
    #                        empty string "".
    # default_search_field:: This specifies the field or fields that will be 
    #                        searched by the query parser. You can use a
    #                        string to specify one field, eg, "title". Or you
    #                        can specify multiple fields with a String -
    #                        "title|content" - or with an Array - ["title",
    #                        "content"]. This defaults to the value passed in
    #                        for default_field. If default_field is nil then
    #                        the default is "*" which signifies all fields in
    #                        the index.
    # analyzer::             Sets the default analyzer for the index. This is
    #                        used by both the IndexWriter and the QueryParser
    #                        to tokenize the input. The default is the
    #                        StandardAnalyzer.
    # dir::                  This is an Ferret::Store::Directory object. This
    #                        can be useful if you have an already existing
    #                        in-memory index which you'd like to read with
    #                        this class. If you want to create a new index,
    #                        you are better off passing in a path.
    # close_dir::            This specifies whether you would this class to
    #                        close the index directory when this class is
    #                        closed. This only has any meaning when you pass
    #                        in a directory object in the *dir* option, in
    #                        which case it defaults to false. Otherwise it is
    #                        always true.
    # occur_default::        Set to either BooleanClause::Occur::SHOULD
    #                        (default) or BooleanClause::Occur::MUST to
    #                        specify the default Occur operator.
    # wild_lower::           Set to false if you don't want the terms in fuzzy
    #                        and wild queries to be set to lower case. You
    #                        should do this if your analyzer doesn't downcase.
    #                        The default is true.
    # default_slop::         Set the default slop for phrase queries. This
    #                        defaults to 0.
    # key::                  Expert: This should only be used if you really
    #                        know what you are doing. Basically you can set a
    #                        field or an array of fields to be the key for the
    #                        index. So if you add a document with a same key
    #                        as an existing document, the existing document will
    #                        be replaced by the new object. This will slow
    #                        down indexing so it should not be used if
    #                        performance is a concern.
    # use_compound_file::    Uses a compound file to store the index. This
    #                        prevents an error being raised for having too
    #                        many files open at the same time. The default is
    #                        true but performance is better if this is set to
    #                        false.
    # handle_parse_errors::  Set this to true if you want the QueryParser to
    #                        degrade gracefully on errors. If the query parser
    #                        fails to parse this query, it will try to parse
    #                        it as a straight boolean query on the default
    #                        field ignoring all query punctuation. If this
    #                        fails, it will return an empty TermQuery. If you
    #                        use this and you need to know why your query
    #                        isn't working you can use the Query#to_s method
    #                        on the query returned to see what is happening to
    #                        your query.  This defualts to true. If you set it
    #                        to false a QueryParseException is raised on a
    #                        query parse error.
    # auto_flush::           Set this option to true if you want the index
    #                        automatically flushed every time you do a write
    #                        (includes delete) to the index. This is useful if
    #                        you have multiple processes accessing the index
    #                        and you don't want lock errors. This is set to
    #                        false by default.
    # 
    # Some examples;
    #
    #   index = Index::Index.new(:analyzer => WhiteSpaceAnalyzer.new())
    #
    #   index = Index::Index.new(:path => '/path/to/index',
    #                            :create_if_missing => false,
    #                            :auto_flush => true)
    #
    #   index = Index::Index.new(:dir => directory,
    #                            :close_dir => false
    #                            :default_slop => 2,
    #                            :handle_parse_errors => false)
    #   
    def initialize(options = {})
      super()

      options[:default_search_field] &&= options[:default_search_field].to_s
      options[:default_field] &&= options[:default_field].to_s
      options[:create_if_missing] = true if options[:create_if_missing].nil? 
      @key = [options[:key]].flatten if options[:key]

      if options[:path]
        @dir = FSDirectory.new(options[:path], options[:create])
        options[:close_dir] = true
      elsif options[:dir]
        @dir = options[:dir]
      else
        options[:create] = true # this should always be true for a new RAMDir
        @dir = RAMDirectory.new
      end

      @dir.synchronize do
        @options = options
        @writer = IndexWriter.new(@dir, options) # create the index if need be
        options[:analyzer] = @analyzer = @writer.analyzer
        @writer.close
        @writer = nil
        @has_writes = false
        @reader = nil
        @options.delete(:create) # only want to create the first time if at all
        @close_dir = @options.delete(:close_dir) || false # we'll hold this here
        @auto_flush = @options[:auto_flush] || false
        @default_search_field = (@options[:default_search_field] || \
                                 @options[:default_field] || "*")
        @default_field = @options[:default_field] || ""
        @options[:handle_parse_errors] = true if @options[:handle_parse_errors].nil?
        @open = true
        @qp = nil
      end
    end

    # Closes this index by closing its associated reader and writer objects.
    def close
      @dir.synchronize do
        if not @open
          raise "tried to close an already closed directory"
        end
        @reader.close() if @reader
        @writer.close() if @writer
        @dir.close()

        @open = false
      end
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
    protected :reader, :writer, :searcher

    # Adds a document to this index, using the provided analyzer instead of
    # the local analyzer if provided.  If the document contains more than
    # IndexWriter::MAX_FIELD_LENGTH terms for a given field, the remainder are
    # discarded.
    #
    # There are three ways to add a document to the index. 
    # To add a document you can simply add a string or an array of strings.
    # This will store all the strings in the "" (ie empty string) field
    # (unless you specify the default_field when you create the index).
    #
    #   index << "This is a new document to be indexed"
    #   index << ["And here", "is another", "new document", "to be indexed"]
    # 
    # But these are pretty simple documents. If this is all you want to index
    # you could probably just use SimpleSearch. So let's give our documents
    # some fields;
    # 
    #   index << {:title => "Programming Ruby", :content => "blah blah blah"}
    #   index << {:title => "Programming Ruby", :content => "yada yada yada"}
    # 
    # Or if you are indexing data stored in a database, you'll probably want
    # to store the id;
    # 
    #   index << {:id => row.id, :title => row.title, :date => row.date}
    # 
    # The methods above while store all of the input data as well tokenizing
    # and indexing it. Sometimes we won't want to tokenize (divide the string
    # into tokens) the data. For example, we might want to leave the title as
    # a complete string and only allow searchs for that complete string.
    # Sometimes we won't want to store the data as it's already stored in the
    # database so it'll be a waste to store it in the index. Or perhaps we are
    # doing without a database and using Ferret to store all of our data, in
    # which case we might not want to index it. For example, if we are storing
    # images in the index, we won't want to index them. All of this can be
    # done using Ferret's Ferret::Document module.  eg;
    # 
    #   include Ferret::Document
    #   doc = Document.new
    #   doc << Field.new("id",    row.id,    Field::Store::NO,  Field::Index::UNTOKENIZED)
    #   doc << Field.new("title", row.title, Field::Store::YES, Field::Index::UNTOKENIZED)
    #   doc << Field.new("data",  row.data,  Field::Store::YES, Field::Index::TOKENIZED)
    #   doc << Field.new("image", row.image, Field::Store::YES, Field::Index::NO)
    #   index << doc
    # 
    # You can also compress the data that you are storing or store term
    # vectors with the data. Read more about this in Ferret::Document::Field.
    def add_document(doc, analyzer = nil)
      @dir.synchronize do
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

        # delete existing documents with the same key
        if @key
          query = @key.map {|field| "+#{field}:#{fdoc[field]}" }.join(" ")
          query_delete(query)
        end

        ensure_writer_open()
        @has_writes = true
        @writer.add_document(fdoc, analyzer || @writer.analyzer)
        flush() if @auto_flush
      end
    end
    alias :<< :add_document

    # The main search method for the index. You need to create a query to
    # pass to this method. You can also pass a hash with one or more of the
    # following; {filter, num_docs, first_doc, sort}
    #
    # query::      The query to run on the index
    # filter::     Filters docs from the search result
    # first_doc::  The index in the results of the first doc retrieved.
    #              Default is 0
    # num_docs::   The number of results returned. Default is 10
    # sort::       An array of SortFields describing how to sort the results.
    def search(query, options = {})
      @dir.synchronize do
        return do_search(query, options)
      end
    end

    # See Index#search
    #
    # This method yields the doc and score for each hit.
    # eg.
    #   index.search_each() do |doc, score|
    #     puts "hit document number #{doc} with a score of #{score}"
    #   end
    #
    # returns:: The total number of hits.
    def search_each(query, options = {}) # :yield: doc, score
      @dir.synchronize do
        hits = do_search(query, options)
        hits.score_docs.each do |score_doc|
          yield score_doc.doc, score_doc.score
        end
        return hits.total_hits
      end
    end

    # Retrieve the document referenced by the document number +id+, if id is
    # an integer or the first document with term +id+ if +id+ is a term.
    #
    # id:: The number of the document to retrieve, or the term used as the id
    #      for the document we wish to retrieve
    def doc(id)
      @dir.synchronize do
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
    end
    alias :[] :doc
    
    # Delete the document referenced by the document number +id+ if +id+ is an
    # integer or all of the documents which have the term +id+ if +id+ is a
    # term..
    #
    # id:: The number of the document to delete
    def delete(id)
      @dir.synchronize do
        cnt = 0
        ensure_reader_open()
        if id.is_a?(String)
          t = Term.new("id", id.to_s)
          cnt = @reader.delete_docs_with_term(t)
        elsif id.is_a?(Term)
          cnt = @reader.delete_docs_with_term(id)
        elsif id.is_a?(Integer)
          cnt = @reader.delete(id)
        else
          raise ArgumentError, "Cannot delete for id of type #{id.class}"
        end
        flush() if @auto_flush
        return cnt
      end
    end

    # Delete all documents returned by the query.
    # 
    # query:: The query to find documents you wish to delete. Can either be a
    #         string (in which case it is parsed by the standard query parser)
    #         or an actual query object.
    def query_delete(query)
      @dir.synchronize do
        ensure_searcher_open()
        query = process_query(query)
        @searcher.search_each(query) do |doc, score|
          @reader.delete(doc)
        end
        flush() if @auto_flush
      end
    end

    # Returns true if document +n+ has been deleted 
    def deleted?(n)
      @dir.synchronize do 
        ensure_reader_open()
        return @reader.deleted?(n) 
      end
    end

    # Update the document referenced by the document number +id+ if +id+ is an
    # integer or all of the documents which have the term +id+ if +id+ is a
    # term..
    #
    # id::      The number of the document to update. Can also be a string
    #           representing the value in the +id+ field or a term to match.
    # new_val:: The values we are updating. This can be a string in which case
    #           the default field is updated, or it can be a hash, in which
    #           case, all fields in the hash are updated. You can also pass a
    #           full Document object, which will completely replace the
    #           documents you remove.
    def update(id, new_val)
      @dir.synchronize do
        if id.is_a?(String)
          query_update("id:#{id}", new_val)
        elsif id.is_a?(Term)
          query_update(TermQuery.new(id), new_val)
        elsif id.is_a?(Integer)
          ensure_reader_open()
          document = doc(id)
          if new_val.is_a?(Hash)
            new_val.each_pair {|name, content| document[name] = content.to_s}
          elsif new_val.is_a?(Document)
            document = new_val
          else
            document[@options[:default_field]] = new_val.to_s
          end          
          @reader.delete(id)
          ensure_writer_open()
          @writer.add_document(document)
        else
          raise ArgumentError, "Cannot update for id of type #{id.class}"
        end
        flush() if @auto_flush
      end
    end

    # Update all the documents returned by the query.
    #
    # query::   The query to find documents you wish to update. Can either be
    #           a string (in which case it is parsed by the standard query
    #           parser) or an actual query object.
    # new_val:: The values we are updating. This can be a string in which case
    #           the default field is updated, or it can be a hash, in which
    #           case, all fields in the hash are updated. You can also pass a
    #           full Document object, which will completely replace the
    #           documents you remove. You should be careful when passing a
    #           whole document to be sure that your query will return one and
    #           only result.
    def query_update(query, new_val)
      @dir.synchronize do
        ensure_searcher_open()
        docs_to_add = []
        query = process_query(query)
        @searcher.search_each(query) do |id, score|
          document = doc(id)
          if new_val.is_a?(Hash)
            new_val.each_pair {|name, content| document[name] = content.to_s}
          elsif new_val.is_a?(Document)
            document = new_val
          else
            document[@options[:default_field]] = new_val.to_s
          end
          docs_to_add << document
          @reader.delete(id)
        end
        ensure_writer_open()
        docs_to_add.each do |document|
          @writer.add_document(document)
        end
        flush() if @auto_flush
      end
    end

    # Returns true if any documents have been deleted since the index was last
    # flushed.
    def has_deletions?()
      @dir.synchronize do
        ensure_reader_open()
        return @reader.has_deletions?
      end
    end
    
    # Returns true if any documents have been added to the index since the
    # last flush.
    def has_writes?()
      return @has_writes
    end

    # Flushes all writes to the index. This will not optimize the index but it
    # will make sure that all writes are written to it.
    #
    # NOTE: this is not necessary if you are only using this class. All writes
    # will automatically flush when you perform an operation that reads the
    # index.
    def flush()
      @dir.synchronize do
        @reader.close if @reader
        @writer.close if @writer
        @reader = nil
        @writer = nil
        @searcher = nil
      end
    end

    # optimizes the index. This should only be called when the index will no
    # longer be updated very often, but will be read a lot.
    def optimize()
      @dir.synchronize do
        ensure_writer_open()
        @writer.optimize()
        flush()
      end
    end

    # returns the number of documents in the index
    def size()
      @dir.synchronize do
        ensure_reader_open()
        return @reader.num_docs()
      end
    end

    # Merges all segments from an index or an array of indexes into this
    # index. You can pass a single Index::Index, Index::Reader,
    # Store::Directory or an array of any single one of these.
    #
    # This may be used to parallelize batch indexing. A large document
    # collection can be broken into sub-collections. Each sub-collection can
    # be indexed in parallel, on a different thread, process or machine and
    # perhaps all in memory. The complete index can then be created by
    # merging sub-collection indexes with this method.
    #
    # After this completes, the index is optimized.
    def add_indexes(indexes)
      @dir.synchronize do
        indexes = [indexes].flatten   # make sure we have an array
        return if indexes.size == 0 # nothing to do
        if indexes[0].is_a?(Index)
          readers = indexes.map {|index| index.reader }
          indexes = readers
        end

        if indexes[0].is_a?(IndexReader)
          ensure_reader_open
          indexes.delete(@reader) # we don't want to merge with self
          ensure_writer_open
          @writer.add_indexes_readers(indexes)
        elsif indexes[0].is_a?(Ferret::Store::Directory)
          indexes.delete(@dir) # we don't want to merge with self
          ensure_writer_open
          @writer.add_indexes(indexes)
        else
          raise ArgumentError, "Unknown index type when trying to merge indexes"
        end
      end
    end

    # This is a simple utility method for saving an in memory or RAM index to
    # the file system. The same thing can be achieved by using the
    # Index::Index#add_indexes method and you will have more options when
    # creating the new index, however this is a simple way to turn a RAM index
    # into a file system index.
    #
    # directory:: This can either be a Store::Directory object or a string
    #             representing the path to the directory where you would
    #             like to store the the index.
    #
    # create::    True if you'd like to create the directory if it doesn't
    #             exist or copy over an existing directory. False if you'd
    #             like to merge with the existing directory. This defaults to
    #             false.
    def persist(directory, create = true)
      synchronize do
        flush()
        old_dir = @dir
        if directory.is_a?(String)
          @dir = FSDirectory.new(directory, create)
          @options[:close_dir] = true
        elsif directory.is_a?(Ferret::Store::Directory)
          @dir = directory
        end
        ensure_writer_open
        @writer.add_indexes([old_dir])
      end
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

      # returns the new reader if one is opened
      def ensure_reader_open()
        raise "tried to use a closed index" if not @open
        if @reader
          if not @reader.latest?
            return @reader = IndexReader.open(@dir, false)
          end
        else
          if @writer
            @writer.close
            @writer = nil
          end
          return @reader = IndexReader.open(@dir, false)
        end
        return false
      end

      def ensure_searcher_open()
        raise "tried to use a closed index" if not @open
        if ensure_reader_open() or not @searcher
          @searcher = IndexSearcher.new(@reader)
        end
      end

    private
      def do_search(query, options)
        ensure_searcher_open()
        query = process_query(query)

        return @searcher.search(query, options)
      end

      def process_query(query)
        if query.is_a?(String)
          if @qp.nil?
            @qp = Ferret::QueryParser.new(@default_search_field, @options)
          end
          # we need to set this ever time, in case a new field has been added
          @qp.fields = @reader.get_field_names.to_a
          query = @qp.parse(query)
        end
        return query
      end
  end
end
