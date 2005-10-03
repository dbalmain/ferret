require 'monitor'

module Ferret::Index
  # IndexReader is an abstract class, providing an interface for accessing an
  # index.  Search of an index is done entirely through this abstract interface,
  # class which implements it is searchable.
  #
  # Concrete subclasses of IndexReader are usually constructed with a call to
  # one of the static +open()+ methods, e.g. +#open(String)+.
  #
  # For efficiency, in this API documents are often referred to via
  # _document numbers_, non-negative integers which each name a unique
  # document in the index.  These document numbers are ephemeral -= 1they may change
  # as documents are added to and deleted from an index.  Clients should thus not
  # rely on a given document having the same number between sessions.
  # 
  # An IndexReader can be opened on a directory for which an IndexWriter is
  # opened already, but it cannot be used to delete documents from the index then.
  class IndexReader 
    include MonitorMixin
    
    # This array contains all filename extensions used by Lucene's index files, with
    # one exception, namely the extension made up from +.f+ + a number.
    # Also note that two of Lucene's files (+deletable+ and
    # +segments+) don't have any filename extension.
    FILENAME_EXTENSIONS = ["cfs",
                           "fnm",
                           "fdx",
                           "fdt",
                           "tii",
                           "tis",
                           "frq",
                           "prx",
                           "del",
                           "tvx",
                           "tvd",
                           "tvf",
                           "tvp"]

    attr_reader :directory
    
    class FieldOption < Parameter
      # all fields
      ALL = FieldOption.new("ALL")
      # all indexed fields
      INDEXED = FieldOption.new("INDEXED")
      # all fields which are not indexed
      UNINDEXED = FieldOption.new("UNINDEXED")
      # all fields which are indexed with termvectors enables
      INDEXED_WITH_TERM_VECTOR = FieldOption.new("INDEXED_WITH_TERM_VECTOR")
      # all fields which are indexed but don't have termvectors enabled
      INDEXED_NO_TERM_VECTOR = FieldOption.new("INDEXED_NO_TERM_VECTOR")
      # all fields where termvectors are enabled. Please note that only standard
      # termvector fields are returned
      TERM_VECTOR = FieldOption.new("TERM_VECTOR")
      # all field with termvectors wiht positions enabled
      TERM_VECTOR_WITH_POSITION = FieldOption.new("TERM_VECTOR_WITH_POSITION")
      # all fields where termvectors with offset position are set
      TERM_VECTOR_WITH_OFFSET = FieldOption.new("TERM_VECTOR_WITH_OFFSET")
      # all fields where termvectors with offset and position values set
      TERM_VECTOR_WITH_POSITION_OFFSET =
        FieldOption.new("TERM_VECTOR_WITH_POSITION_OFFSET")
    end
    
    # directory:: Directory where IndexReader files reside.
    # segment_infos:: Used for write-l
    # close_directory:: close the directory when the index reader is closed
    def initialize(directory, segment_infos = nil,
                   close_directory = false, directory_owner = false)
      super()
      @directory = directory
      @close_directory = close_directory
      @segment_infos = segment_infos
      @directory_owner = directory_owner

      @has_changes = false
      @stale = false
      @write_lock = nil

      #ObjectSpace.define_finalizer(self, lambda { |id| @write_lock.release() if @write_lock})
    end

    # Returns an index reader to read the index in the directory
    def IndexReader.open(directory, close_directory = true)
      directory.synchronize do # in- & inter-process sync
        commit_lock = directory.make_lock(IndexWriter::COMMIT_LOCK_NAME)
        commit_lock.while_locked() do
          infos = SegmentInfos.new()
          infos.read(directory)
          if (infos.size() == 1) # index is optimized
            return SegmentReader.get(infos, infos[0], close_directory)
          end
          readers = Array.new(infos.size)
          infos.size.times do |i|
            readers[i] = SegmentReader.get(infos, infos[i], close_directory)
          end

          return MultiReader.new(directory, infos, close_directory, readers)
        end
      end
    end

    # Reads version number from segments files. The version number counts the
    # number of changes of the index.
    # 
    # directory:: where the index resides.
    # returns:: version number.
    # raises:: IOError if segments file cannot be read.
    def get_current_version(directory)
      return SegmentInfos.read_current_version(directory)
    end

    # Return an array of term vectors for the specified document.  The array
    # contains a vector for each vectorized field in the document.  Each vector
    # contains terms and frequencies for all terms in a given vectorized field.
    # If no such fields existed, the method returns nil. The term vectors that
    # are returned my either be of type TermFreqVector or of type
    # TermDocPosEnumVector if positions or offsets have been stored.
    # 
    # doc_number:: document for which term vectors are returned
    # returns:: array of term vectors. May be nil if no term vectors have been
    #           stored for the specified document.
    # raises:: IOError if index cannot be accessed
    #
    # See Field.TermVector
    def get_term_vectors(doc_number)
      raise NotImplementedError
    end
           

    
    # Return a term vector for the specified document and field. The returned
    # vector contains terms and frequencies for the terms in the specified
    # field of this document, if the field had the storeTermVector flag set. If
    # termvectors had been stored with positions or offsets, a
    # TermDocPosEnumVector is returned.
    # 
    # doc_number:: document for which the term vector is returned
    # field:: field for which the term vector is returned.
    # returns:: term vector May be nil if field does not exist in the specified
    #           document or term vector was not stored.
    # raises:: IOError if index cannot be accessed
    # See Field.TermVector
    def get_term_vector(doc_number, field)
      raise NotImplementedError
    end
           
   
    # Returns +true+ if an index exists at the specified directory.  If the
    # directory does not exist or if there is no index in it.
    #
    # directory:: the directory to check for an index
    # returns:: +true+ if an index exists; +false+ otherwise
    # raises:: IOError if there is a problem with accessing the index
    def index_exists?(directory)
      return directory.exists?("segments")
    end

    # Returns the number of documents in this index. 
    def num_docs()
      raise NotImplementedError
    end

    # Returns one greater than the largest possible document number.
    #
    # This may be used to, e.g., determine how big to allocate an array which
    # will have an element for every document number in an index.
    def max_doc()
      raise NotImplementedError
    end

    # Returns the stored fields of the +n+<sup>th</sup>
    # +Document+ in this index. 
    def document(n)
      raise NotImplementedError
    end

    # Returns true if document _n_ has been deleted 
    def deleted?(n)
      raise NotImplementedError
    end

    # Returns true if any documents have been deleted 
    def has_deletions?()
      raise NotImplementedError
    end
    
    # Returns the byte-encoded normalization factor for the named field of
    # every document.  This is used by the search code to score documents.
    # 
    # See Field#boost
    def norms(field, bytes=nil, offset=nil)
      raise NotImplementedError
    end

    # Expert: Resets the normalization factor for the named field of the named
    # document.  The norm represents the product of the field's Field#boost and
    # its Similarity#length_norm length normalization.  Thus, to preserve the
    # length normalization values when resetting this, one should base the new
    # value upon the old.
    # 
    # See #norms
    # See Similarity#decode_norm
    def set_norm(doc, field, value)
      synchronize do
        value = Similarity.encode_norm(value) if value.is_a? Float
        if(@directory_owner)
          aquire_write_lock()
        end
        do_set_norm(doc, field, value)
        @has_changes = true
      end
    end
            
    # Implements set_norm in subclass.
    def do_set_norm(doc, field, value) 
      raise NotImplementedError
    end
           
    # Returns an enumeration of all the terms in the index.
    # Each term is greater than all that precede it in the enumeration.
    def terms()
      raise NotImplementedError
    end

    # Returns an enumeration of all terms after a given term.
    #
    # Each term is greater than all that precede it in the enumeration.
    def terms_from(t)
      raise NotImplementedError
    end

    # Returns the number of documents containing the term +t+. 
    def doc_freq(t)
      raise NotImplementedError
    end

    # Returns an enumeration of all the documents which contain +term+. For each
    # document, the document number, the frequency of the term in that document
    # is also provided, for use in search scoring.  Thus, this method implements
    # the mapping:
    # 
    #   Term => <doc_num, freq><sup>*</sup>
    # 
    # The enumeration is ordered by document number.  Each document number is
    # greater than all that precede it in the enumeration.
    def term_docs_for(term)
      term_docs = term_docs()
      term_docs.seek(term)
      return term_docs
    end

    # Returns an unpositioned TermDocEnum enumerator. 
    def term_docs()
      raise NotImplementedError
    end

    # Returns an enumeration of all the documents which contain
    # +term+.  For each document, in addition to the document number
    # and frequency of the term in that document, a list of all of the ordinal
    # positions of the term in the document is available.  Thus, this method
    # implements the mapping:
    #
    #   Term => <doc_num, freq, < pos<sub>1</sub>, pos<sub>2</sub>, ...
    #   pos<sub>freq-1</sub> > > <sup>*</sup>
    #
    # This positional information faciliates phrase and proximity searching.
    # The enumeration is ordered by document number.  Each document number is
    # greater than all that precede it in the enumeration.
    def term_positions_for(term)
      term_positions = term_positions()
      term_positions.seek(term)
      return term_positions
    end

    # Returns an unpositioned @link TermDocPosEnumendenumerator. 
    def term_positions()
      raise NotImplementedError
    end

    # Tries to acquire the WriteLock on this directory.
    #
    # This method is only valid if this IndexReader is directory owner.
    # 
    # raises:: IOError If WriteLock cannot be acquired.
    def aquire_write_lock()
      if @stale
        raise IOError, "IndexReader out of date and no longer valid for delete, undelete, or set_norm operations"
      end

      if (@write_lock == nil) 
        @write_lock = directory.make_lock(IndexWriter.WRITE_LOCK_NAME)
        if not @write_lock.obtain(IndexWriter.WRITE_LOCK_TIMEOUT) # obtain write lock
          raise IOError, "Index locked for write: " + @write_lock
        end

        # we have to check whether index has changed since this reader was opened.
        # if so, this reader is no longer valid for deletion
        if (SegmentInfos.read_current_version(directory) > segment_infos.version()) 
          @stale = true
          @write_lock.release()
          @write_lock = nil
          raise IOError, "IndexReader out of date and no longer valid for delete, undelete, or set_norm operations"
        end
      end
    end
    
    # Deletes the document numbered +doc_num+.  Once a document is deleted it
    # will not appear in TermDocEnum or TermPostitions enumerations.  Attempts to
    # read its field with the @link #documentend method will result in an error.
    # The presence of this document may still be reflected in the @link
    # #docFreqendstatistic, though this will be corrected eventually as the
    # index is further modified.
    def delete(doc_num)
      synchronize do
        aquire_write_lock() if @directory_owner
        do_delete(doc_num)
        @has_changes = true
      end
    end

    # Implements deletion of the document numbered +doc_num+.
    # Applications should call @link #delete(int)endor @link #delete(Term)end.
    def do_delete(doc_num)
      raise NotImplementedError
    end

    # Deletes all documents containing +term+.
    # This is useful if one uses a document field to hold a unique ID string for
    # the document.  Then to delete such a document, one merely constructs a
    # term with the appropriate field and the unique ID string as its text and
    # passes it to this method.  Returns the number of documents deleted.  See
    # #delete for information about when this deletion will become effective.
    def delete_docs_with_term(term)
      docs = term_docs(term)
      if (docs == nil) then return 0 end
      n = 0
      begin 
        while (docs.next()) 
          delete(docs.doc())
          n += 1
        end
      ensure 
        docs.close()
      end
      return n
    end

    # Undeletes all documents currently marked as deleted in this index.
    def undelete_all()
      synchronize do
        aquire_write_lock() if @directory_owner
        do_undelete_all()
        @has_changes = true
      end
    end
    
    # Closes files associated with this index.
    # Also saves any new deletions to disk.
    # No other methods should be called after this has been called.
    def close()
      synchronize do
        commit()
        do_close()
        directory.close() if @close_directory
      end
    end

    protected 

    # Implements actual undelete_all() in subclass. 
    def do_undelete_all()
      raise NotImplementedError
    end

    # Commit changes resulting from delete, undelete_all, or set_norm operations
    # 
    # raises:: IOError
    def commit()
      synchronize do
        if @has_changes
          if @directory_owner
            directory.synchronize do # in- & inter-process sync
              commit_lock = directory.make_lock(IndexWriter.COMMIT_LOCK_NAME)
              commit_lock.while_locked do
                do_commit()
                @segment_infos.write(@directory)
              end
            end
            if (@write_lock != nil) 
              @write_lock.release()  # release write lock
              @write_lock = nil
            end
          else
            do_commit()
          end
        end
        @has_changes = false
      end
    end
    
    # Implements commit. 
    def do_commit()
      raise NotImplementedError
    end
    

    # Implements close. 
    def do_close()
      raise NotImplementedError
    end

    # Get a list of unique field names that exist in this index and have the
    # specified field option information.
    # fld_option:: specifies which field option should be available for the
    #              returned fields
    # returns:: Collection of Strings indicating the names of the fields.
    # See IndexReader.FieldOption
    def get_field_names()
      raise NotImplementedError
    end

    # Returns +true+ iff the index in the named directory is
    # currently locked.
    # directory:: the directory to check for a lock
    # raises:: IOError if there is a problem with accessing the index
    def locked?(directory)
      return (directory.make_lock(IndexWriter.WRITE_LOCK_NAME).locked? or
        directory.make_lock(IndexWriter.COMMIT_LOCK_NAME).locked?)
    end

    # Forcibly unlocks the index in the named directory.
    #
    # Caution: this should only be used by failure recovery code,
    # when it is known that no other process nor thread is in fact
    # currently accessing this index.
    def unlock(directory)
      directory.make_lock(IndexWriter.WRITE_LOCK_NAME).release
      directory.make_lock(IndexWriter.COMMIT_LOCK_NAME).release
    end
  end
end
