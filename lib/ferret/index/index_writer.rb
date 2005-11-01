require 'ferret/search/similarity'

module Ferret
module Index
#module Ferret::Index

  require "monitor"

  # An IndexWriter creates and maintains an index.
  #
  # The third argument to new  determines whether a new index is created,
  # or whether an existing index is opened for the addition of new documents.
  #
  # In either case, documents are added with the add_document method.  When
  # finished adding documents, close should be called.
  #
  # If an index will not have more documents added for a while and optimal search
  # performance is desired, then the optimize method should be called before the
  # index is closed.
  #
  # Opening an IndexWriter creates a lock file for the directory in use.
  # Trying to open another IndexWriter on the same directory will lead to
  # an IOError. The IOError is also thrown if an IndexReader on the same
  # directory is used to delete documents from the index.
  class IndexWriter
    include MonitorMixin
    include ObjectSpace

    WRITE_LOCK_TIMEOUT = 1
    COMMIT_LOCK_TIMEOUT = 10
    WRITE_LOCK_NAME = "write.lock"
    COMMIT_LOCK_NAME = "commit.lock"
    DEFAULT_MERGE_FACTOR = 10 
    DEFAULT_MIN_MERGE_DOCS = 10
    DEFAULT_MAX_MERGE_DOCS = 0x7fffffff
    DEFAULT_MAX_FIELD_LENGTH = 10000
    DEFAULT_TERM_INDEX_INTERVAL = 128

    attr_accessor :use_compound_file, :similarity, :term_index_interval,
      :max_merge_docs, :max_field_length, :min_merge_docs, :info_stream
    attr_reader :analyzer, :directory, :merge_factor, :segment_infos 
    alias :max_buffered_docs :min_merge_docs
    alias :max_buffered_docs= :min_merge_docs=

    def merge_factor=(mf)
      raise ArgumentError, "merge factor cannot be less than 2" if (mf < 2)
      @merge_factor = mf
    end

    # Constructs an IndexWriter for the index in +dir+.
    # Text will be analyzed with +analyzer+.  If +create+
    # is true, then a new, empty index will be created in
    # +dir+, replacing the index already there, if any.
    # NOTE:: all options are passed in a hash.
    #
    # dir::               the index directory
    #
    # == Options
    # 
    # analyzer::          the analyzer to use. Defaults to StandardAnalyzer.
    # create::            +true+ to create the index or overwrite the existing
    #                     one +false+ to append to the existing index
    # create_if_missing:: +true+ to create the index if it's missing
    #                     +false+ to throw an IOError if it's missing
    # close_dir::         This specifies whether you would this class to close
    #                     the index directory when this class is closed. The
    #                     default is false.
    # use_compound_file:: Use a compound file to store the index. This is
    #                     slower than using multiple files but it prevents the
    #                     too many files open error. This defaults to true.
    def initialize(dir = nil, options = {})
      super()
      create = options[:create] || false
      create_if_missing = options[:create_if_missing] || false

      if dir.nil?
        @directory = Ferret::Store::RAMDirectory.new
      elsif dir.is_a?(String)
        @directory = Ferret::Store::FSDirectory.new(dir, create)
      else
        @directory = dir
      end
      @close_dir = options[:close_dir] || false
      @use_compound_file = (options[:use_compound_file] != false) # ie default true
      @analyzer = options[:analyzer] || Ferret::Analysis::StandardAnalyzer.new
      @merge_factor = DEFAULT_MERGE_FACTOR
      @min_merge_docs = DEFAULT_MIN_MERGE_DOCS
      @max_merge_docs = DEFAULT_MAX_MERGE_DOCS
      @max_field_length = DEFAULT_MAX_FIELD_LENGTH
      @term_index_interval = DEFAULT_TERM_INDEX_INTERVAL

      @similarity = Search::Similarity.default
      @segment_infos = SegmentInfos.new()
      @ram_directory = Ferret::Store::RAMDirectory.new()

      # Make sure that the lock is released when this object is destroyed
      define_finalizer(self, proc { |id| @write_lock.release() if @write_lock})
   
      @write_lock = @directory.make_lock(WRITE_LOCK_NAME)
      @write_lock.obtain(WRITE_LOCK_TIMEOUT) # obtain write lock

      @directory.synchronize() do # in- & inter-process sync
        @directory.make_lock(COMMIT_LOCK_NAME).while_locked(COMMIT_LOCK_TIMEOUT) do
          if (create)
            @segment_infos.write(@directory)
          else
            begin
              @segment_infos.read(@directory)
            rescue Exception => e
              if options[:create_if_missing]
                @segment_infos.write(@directory)
              else
                @write_lock.release() # obtain write lock
                raise e
              end
            end
          end 
        end
      end

      @info_stream = nil
    end

    # Flushes all changes to an index and closes all associated files.
    def close()
      synchronize() do
        flush_ram_segments()
        @ram_directory.close()
        @write_lock.release() if @write_lock # release write lock
        @write_lock = nil
        if(@close_dir)
          @directory.close()
        end
      end
    end

    # Returns the number of documents currently in this index.
    def doc_count()
      synchronize() do
        count = 0
        @segment_infos.each { |si| count += si.doc_count() }
        return count
      end
    end

    # Adds a document to this index, using the provided analyzer instead of the
    # local analyzer if provided.  If the document contains more than
    # #max_field_length terms for a given field, the remainder are
    # discarded.
    def add_document(doc, analyzer=@analyzer)
      dw = DocumentWriter.new(@ram_directory,
                              analyzer,
                              @similarity,
                              @max_field_length,
                              @term_index_interval)
      dw.info_stream = @info_stream
      segment_name = new_segment_name()
      dw.add_document(segment_name, doc)
      synchronize() do
        @segment_infos << SegmentInfo.new(segment_name, 1, @ram_directory)
        maybe_merge_segments()
      end
    end
    alias :<< :add_document

    def segments_counter()
      return segment_infos.counter
    end

    # Merges all segments together into a single segment, optimizing an index
    # for search.
    def optimize()
      synchronize() do
        flush_ram_segments()
        while (@segment_infos.size() > 1 ||
                (@segment_infos.size() == 1 &&
                  (SegmentReader.has_deletions?(@segment_infos[0]) ||
                    (@segment_infos[0].directory != @directory) ||
                      (@use_compound_file &&
                        (!SegmentReader.uses_compound_file?(@segment_infos[0]) ||
                          SegmentReader.has_separate_norms?(@segment_infos[0]))))))
          min_segment = @segment_infos.size() - @merge_factor
          merge_segments(min_segment < 0 ? 0 : min_segment)
        end
      end
    end

    # Merges all segments from an array of indexes into this index.
    #
    # This may be used to parallelize batch indexing.  A large document
    # collection can be broken into sub-collections.  Each sub-collection can be
    # indexed in parallel, on a different thread, process or machine.  The
    # complete index can then be created by merging sub-collection indexes
    # with this method.
    #
    # After this completes, the index is optimized.
    def add_indexes(dirs)
      synchronize() do
        optimize()                        # start with zero or 1 seg

        start = @segment_infos.size

        dirs.each do |dir|
          sis = SegmentInfos.new()        # read infos from dir
          sis.read(dir)
          sis.each do |si|
            @segment_infos << si
          end
        end

        # merge newly added segments in log(n) passes
        while (@segment_infos.size > start + @merge_factor)
          (start+1 ... @segment_infos.size).each do |base|
            last = [@segment_infos.size(),  (base + @merge_factor)].min
            if (last - base > 1)
              merge_segments(base, last);
            end
          end
        end

        optimize() # final cleanup
      end
    end

    # Merges the provided indexes into this index.
    # After this completes, the index is optimized. 
    # The provided IndexReaders are not closed.
    def add_indexes_readers(readers)
      synchronize() do
        segments_to_delete = []
        optimize() # start with zero or 1 seg

        merged_name = new_segment_name()
        merger = SegmentMerger.new(@directory, merged_name, @term_index_interval)

        if (@segment_infos.size() == 1) # add existing index, if any
          s_reader = SegmentReader.get(@segment_infos[0])
          merger << s_reader
          segments_to_delete << s_reader
        end

        readers.each do |reader|
          merger << reader
        end

        doc_count = merger.merge() # merge 'em

        @segment_infos.clear() # pop old infos & add new
        @segment_infos << SegmentInfo.new(merged_name, doc_count, @directory)

        @directory.synchronize() do
          @directory.make_lock(COMMIT_LOCK_NAME).while_locked(COMMIT_LOCK_TIMEOUT) do
            @segment_infos.write(@directory) # commit changes
            delete_segments(segments_to_delete)
          end
        end

        if @use_compound_file
          files_to_delete = merger.create_compound_file(merged_name + ".tmp")
          @directory.synchronize() do # in- & inter-process sync
            @directory.make_lock(COMMIT_LOCK_NAME).while_locked(COMMIT_LOCK_TIMEOUT) do
              # make compound file visible for SegmentReaders
              @directory.rename(merged_name + ".tmp", merged_name + ".cfs")
              # delete now unused files of segment
              delete_files_and_write_undeletable(files_to_delete)
            end
          end
        end

        optimize()
      end
    end



    private

      # Use compound file setting. Defaults to true, minimizing the number of
      # files used.  Setting this to false may improve indexing performance, but
      # may also cause file handle problems.
      @use_compound_file = true

      # The maximum number of terms that will be indexed for a single field in a
      # document.  This limits the amount of memory required for indexing, so that
      # collections with very large files will not crash the indexing process by
      # running out of memory.
      #
      # Note that this effectively truncates large documents, excluding from the
      # index terms that occur further in the document.  If you know your source
      # documents are large, be sure to set this value high enough to accomodate
      # the expected size.  If you set it to a really big number, then the only limit
      # is your memory, but you should anticipate an OutOfMemoryError.
      #
      # By default, no more than 10,000 terms will be indexed for a field.
      @max_field_length = DEFAULT_MAX_FIELD_LENGTH

      def new_segment_name()
        # The name will be "_" + seg_counter where seg_counter is stored in
        # radix of 36 which is equal to MAX_RADIX in Java
        synchronize() do
          seg_name = "_" + @segment_infos.counter.to_s(36)
          @segment_infos.counter+=1
          return seg_name
        end
      end

      # Determines how often segment indices are merged by add_document().  With
      # smaller values, less RAM is used while indexing, and searches on
      # unoptimized indices are faster, but indexing speed is slower.  With larger
      # values, more RAM is used during indexing, and while searches on unoptimized
      # indices are slower, indexing is faster.  Thus larger values (> 10) are best
      # for batch index creation, and smaller values (< 10) for indices that are
      # interactively maintained.
      #
      # This must never be less than 2.  The default value is 10.*/
      @merge_factor = DEFAULT_MERGE_FACTOR

      # Determines the minimal number of documents required before the buffered
      # in-memory documents are merging and a new Segment is created.
      # Since Documents are merged in a org.apache.lucene.store.RAMDirectory},
      # large value gives faster indexing.  At the same time, merge_factor limits
      # the number of files open in a FSDirectory.
      #
      #  The default value is 10.*/
      @min_merge_docs = DEFAULT_MIN_MERGE_DOCS


      # Determines the largest number of documents ever merged by add_document().
      # Small values (e.g., less than 10,000) are best for interactive indexing,
      # as this limits the length of pauses while indexing to a few seconds.
      # Larger values are best for batched indexing and speedier searches.
      @max_merge_docs = DEFAULT_MAX_MERGE_DOCS

      # Merges all RAM-resident segments.
      def flush_ram_segments()
        min_segment = @segment_infos.size()-1
        doc_count = 0
        while (min_segment >= 0 &&
                (@segment_infos[min_segment]).directory == @ram_directory)
          doc_count += @segment_infos[min_segment].doc_count
          min_segment -= 1
        end
        if (min_segment < 0 ||                   # add one FS segment?
            (doc_count + @segment_infos[min_segment].doc_count) > @merge_factor ||
            !(@segment_infos[@segment_infos.size-1].directory == @ram_directory))
          min_segment += 1
        end
        if (min_segment >= @segment_infos.size()) then
          return 
        end                                      # none to merge
        merge_segments(min_segment)
      end

      # Incremental segment merger. 
      def maybe_merge_segments()
        target_merge_docs = @min_merge_docs
        while (target_merge_docs <= @max_merge_docs)
          # find segments smaller than current target size
          min_segment = @segment_infos.size() -1
          merge_docs = 0
          while (min_segment >= 0)
            si = @segment_infos[min_segment]
            if (si.doc_count >= target_merge_docs)
              break
            end
            merge_docs += si.doc_count
            min_segment -= 1
          end

          if (merge_docs >= target_merge_docs)   # found a merge to do
            merge_segments(min_segment + 1)
          else
            break
          end

          target_merge_docs *= @merge_factor     # increase target size
        end
      end

      # Pops segments off of @segment_infos stack down to min_segment, merges them,
      # and pushes the merged index onto the top of the @segment_infos stack.
      def merge_segments(min_segment, max_segment = @segment_infos.size)
        segments_to_delete = []
        merged_name = new_segment_name()
        if @info_stream != nil
          @info_stream.print("merging segments from #{min_segment} to #{(max_segment - 1)}\n")
        end
        merger = SegmentMerger.new(@directory, merged_name, @term_index_interval)

        (min_segment ... max_segment).each do |i|
          si = @segment_infos[i]
          if (@info_stream != nil)
            @info_stream.print(" #{si.name} (#{si.doc_count} docs)\n")
          end
          reader = SegmentReader.new(si.directory, si, nil, false, false)
          merger.add(reader)
          if ((reader.directory() == @directory) || # if we own the directory
              (reader.directory() == @ram_directory))
            segments_to_delete << reader   # queue segment for deletion
          end
        end

        merged_doc_count = merger.merge()

        if (@info_stream != nil)
          @info_stream.print(" into #{merged_name} (#{merged_doc_count.to_s} docs)\n")
        end

        (max_segment-1).downto(min_segment) {|i| @segment_infos.delete_at(i) }

        @segment_infos << SegmentInfo.new(merged_name, merged_doc_count, @directory)

        # close readers before we attempt to delete now-obsolete segments
        merger.close_readers()

        @directory.synchronize() do
          @directory.make_lock(COMMIT_LOCK_NAME).while_locked(COMMIT_LOCK_TIMEOUT) do
            @segment_infos.write(@directory)     # commit before deleting
            delete_segments(segments_to_delete)  # delete now-unused segments
          end
        end

        if @use_compound_file
          files_to_delete = merger.create_compound_file(merged_name + ".tmp")
          @directory.synchronize() do # in- & inter-process sync
            @directory.make_lock(COMMIT_LOCK_NAME).while_locked(COMMIT_LOCK_TIMEOUT) do
              # make compound file visible for SegmentReaders
              @directory.rename(merged_name + ".tmp", merged_name + ".cfs")
              # delete now unused files of segment
              delete_files_and_write_undeletable(files_to_delete)
            end
          end
        end

      end

      # Some operating systems (e.g. Windows) don't permit a file to be
      # deleted while it is opened for read (e.g. by another process or
      # thread).  So we assume that when a delete fails it is because the
      # file is open in another process, and queue the file for subsequent
      # deletion.
      def delete_segments(segment_readers)
        deletable = []

        try_to_delete_files(read_deleteable_files(), deletable)
        segment_readers.each do |segment_reader|
          if (segment_reader.directory() == @directory)
            try_to_delete_files(segment_reader.file_names(), deletable)
          else
            # delete other files
            delete_files(segment_reader.file_names(), segment_reader.directory())
          end
        end

        write_deleteable_files(deletable) # note files we can't delete
        # This is a great time to start the garbage collector as all of our
        # ram files have just become free
        GC.start

##############################################################################
#          objs = {}
#          ObjectSpace.each_object do |obj|
#            objs[obj.class] ||= 0
#            objs[obj.class] += 1
#          end
#          File.open('objects.out','a+') do |fh|
#            fh.puts("____________________")
#            fh.puts("____________________")
#            objs.each_pair do |obj, count|
#              fh.puts "#{count}\t#{obj}"
#            end
#          end
##############################################################################

      end

      def delete_files_and_write_undeletable(files)
        deletable = []
        try_to_delete_files(read_deleteable_files(), deletable) # try to delete deleteable
        try_to_delete_files(files, deletable)     # try to delete our files
        write_deleteable_files(deletable)    # note files we can't delete
      end

      def delete_files(file_names, dir)
        file_names.each do |file_name|
          dir.delete(file_name)
        end
      end

      def try_to_delete_files(file_names, deletable)
        file_names.each do |file_name|
          begin
            @directory.delete(file_name) # try to delete each file
          rescue IOError => e
            if (@directory.exists?(file_name))
              if (@info_stream != nil) then @info_stream.print(e.to_s + " Will re-try later.") end
              deletable << file_name # add to deletable
            end
          end
        end
      end

      def read_deleteable_files()
        file_names = []
        if (!@directory.exists?("deletable")) then return file_names end

        input = @directory.open_input("deletable")
        begin
          file_count = input.read_int()
          file_count.times do
            file_names << input.read_string()
          end
        ensure
          input.close()
        end
        return file_names
      end

      def write_deleteable_files(file_names)
        output = @directory.create_output("deleteable.new")
        begin
          output.write_int(file_names.size())
          file_names.each do |file_name|
            output.write_string(file_name)
          end
        ensure
          output.close()
        end
        @directory.rename("deleteable.new", "deletable")
      end
  end
end
end
