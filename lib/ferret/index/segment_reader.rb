module Ferret::Index

  # FIXME: Describe class +SegmentReader+ here.
  # 
  class SegmentReader < IndexReader 

    attr_reader :freq_stream, :prox_stream, :deleted_docs,
      :term_infos, :field_infos, :segment

    def SegmentReader.get(info, infos = nil, close = false)
      return SegmentReader.new(info.directory, info, infos, close, infos!=nil)
    end
    
    def initialize(dir, info, seg_infos, close, owner)
      super(dir, seg_infos, close, owner)
      @segment = info.name

      @cfs_reader = nil
      cfs = directory
      if directory.exists?(@segment + '.cfs') then
        @cfs_reader = CompoundFileReader.new(directory, @segment + '.cfs')
        cfs = @cfs_reader
      end

      @field_infos = FieldInfos.new(cfs, @segment + '.fnm')
      @fields_reader = FieldsReader.new(cfs, @segment, @field_infos)

      @term_infos = TermInfosReader.new(cfs, @segment, @field_infos)
      @deleted_docs = nil
      @deleted_docs_dirty = false
      if SegmentReader.has_deletions?(info) then
        @deleted_docs =
          Ferret::Utils::BitVector.read(directory, @segment + '.del')
      end

      @freq_stream = cfs.open_input(@segment + '.frq')
      @prox_stream = cfs.open_input(@segment + '.prx')
      @norms = {}
      @norms.extend(MonitorMixin)
      open_norms(cfs)

      if @field_infos.has_vectors? then
        @tv_reader_orig = TermVectorsReader.new(cfs, @segment, @field_infos)
      end
    end

    def do_commit()
      if (@deleted_docs_dirty) # re-write deleted 
        @deleted_docs.write(@directory, @segment + '.tmp')
        @directory.rename(@segment + '.tmp', @segment + '.del')
      end
      if(@undelete_all and @directory.exists?(@segment + '.del'))
        @directory.delete(@segment + '.del')
      end
      if (@norms_dirty) # re-write norms 
        @norms.each_value do |norm|
          if norm.dirty? 
            norm.re_write(@directory, @segment, max_doc(), @cfs_reader)
          end
        end
      end
      @deleted_docs_dirty = false
      @norms_dirty = false
      @undelete_all = false
    end
    
    def do_close()
      Thread.current["#{self.object_id}-tv_reader"] = nil # clear the cache
      @fields_reader.close()
      @term_infos.close()

      @freq_stream.close() if @freq_stream
      @prox_stream.close() if @prox_stream

      close_norms()
      
      @tv_reader_orig.close() if @tv_reader_orig
      @cfs_reader.close() if @cfs_reader
    end

    def SegmentReader.has_deletions?(si)
      return si.directory.exists?(si.name + ".del")
    end

    def has_deletions?() 
      return @deleted_docs != nil
    end


    def SegmentReader.uses_compound_file?(si)
      return si.dir.exists?(si.name + ".cfs")
    end
    
    def SegmentReader.has_separate_norms?(si)
      return (si.dir.list.select {|f| f =~ /^#{si.name}\.s/}).size > 0
    end

    def do_delete(doc_num) 
      if (@deleted_docs == nil)
        @deleted_docs = Ferret::Utils::BitVector.new
      end
      @deleted_docs_dirty = true
      @undelete_all = false
      @deleted_docs.set(doc_num)
    end

    def do_undelete_all() 
      @deleted_docs = nil
      @deleted_docs_dirty = false
      @undelete_all = true
    end

    def file_names()
      file_names = []

      IndexFileNames::INDEX_EXTENSIONS.each do |ext|
        name = @segment + "." + ext
        if (@directory.exists?(name))
          file_names << name
        end
      end

      @field_infos.each_with_index do |fi, i|
        if (fi.indexed?)
          if @cfs_reader.nil?
            name = @segment + ".f" + i.to_s
          else
            name = @segment + ".s" + i.to_s
          end
          if (@directory.exists?(name))
            file_names << name
          end
        end
      end
      return file_names
    end

    def terms() 
      return @term_infos.terms()
    end

    def terms_from(t)
      return @term_infos.terms_from(t)
    end

    def get_document(n)
      synchronize do
        if deleted?(n)
          raise ArgumentError, "attempt to access a deleted document"
        end
        return @fields_reader.doc(n)
      end
    end

    def deleted?(n) 
      synchronize do
        return (@deleted_docs != nil and @deleted_docs.get(n))
      end
    end

    def term_docs()
      return SegmentTermDocEnum.new(self)
    end

    def term_positions()
      return SegmentTermDocPosEnum.new(self)
    end

    def doc_freq(t)
      ti = @term_infos.get_term_info(t)
      if (ti != nil)
        return ti.doc_freq
      else
        return 0
      end
    end

    def num_docs() 
      n = max_doc()
      if (@deleted_docs != nil)
        n -= @deleted_docs.count()
      end
      return n
    end

    def max_doc() 
      return @fields_reader.size()
    end

    # See IndexReader#get_field_names
    def get_field_names(field_option = IndexReader::FieldOption::ALL) 
      field_set = Set.new
      @field_infos.each do |fi|
        if (field_option == IndexReader::FieldOption::ALL) 
          field_set.add(fi.name)
        elsif (!fi.indexed? and field_option == IndexReader::FieldOption::UNINDEXED) 
          field_set.add(fi.name)
        elsif (fi.indexed? and field_option == IndexReader::FieldOption::INDEXED) 
          field_set.add(fi.name)
        elsif (fi.indexed? and fi.store_term_vector? == false and
               field_option == IndexReader::FieldOption::INDEXED_NO_TERM_VECTOR) 
          field_set.add(fi.name)
        elsif (fi.store_term_vector? == true and
               fi.store_positions? == false and
               fi.store_offsets? == false and
               field_option == IndexReader::FieldOption::TERM_VECTOR) 
          field_set.add(fi.name)
        elsif (fi.indexed? and fi.store_term_vector? and
               field_option == IndexReader::FieldOption::INDEXED_WITH_TERM_VECTOR) 
          field_set.add(fi.name)
        elsif (fi.store_positions? and fi.store_offsets? == false and
               field_option == IndexReader::FieldOption::TERM_VECTOR_WITH_POSITION) 
          field_set.add(fi.name)
        elsif (fi.store_offsets? and fi.store_positions? == false and
               field_option == IndexReader::FieldOption::TERM_VECTOR_WITH_OFFSET) 
          field_set.add(fi.name)
        elsif (fi.store_offsets? and fi.store_positions? and
               field_option == IndexReader::FieldOption::TERM_VECTOR_WITH_POSITION_OFFSET)
          field_set.add(fi.name)
        end
      end
      return field_set
    end

    def get_norms(field)
      synchronize do
        norm = @norms[field]
        if (norm == nil)               # not an indexed field
          return nil
        end
        if (norm.bytes == nil)         # value not yet read
          bytes = " " * max_doc()
          get_norms_into(field, bytes, 0)
          norm.bytes = bytes           # cache it
        end
        return norm.bytes
      end
    end

    def do_set_norm(doc, field, value)
      norm = @norms[field]
      if (norm == nil)                             # not an indexed field
        return
      end
      norm.dirty = true                            # mark it dirty
      @norms_dirty = true

      get_norms(field)[doc] = value                # set the value
    end

    # Read norms into a pre-allocated array. 
    def get_norms_into(field, bytes, offset)
      synchronize do
        norm = @norms[field]
        return if (norm == nil) # use zeros in array

        if (norm.bytes != nil) # can copy from cache
          bytes[offset, max_doc()] = norm.bytes[0, max_doc()]
          return
        end

        norm_stream = norm.is.clone()
        begin # read from disk
          norm_stream.seek(0)
          norm_stream.read_bytes(bytes, offset, max_doc())
        ensure 
          norm_stream.close()
        end
      end
    end

    def open_norms(cfs_dir)
      @field_infos.each do |fi|
        if (fi.indexed?) 
          # look first if there are separate norms in compound format
          file_name = @segment + ".s" + fi.number.to_s
          d = @directory
          if not d.exists?(file_name)
            file_name = @segment + ".f" + fi.number.to_s
            d = cfs_dir
          end
          @norms[fi.name] = Norm.new(d.open_input(file_name), fi.number)
        end
      end
    end

    def close_norms()
      @norms.synchronize do
        @norms.each_value {|norm| norm.is.close()}
      end
    end
    
    # Create a clone from the initial TermVectorsReader and store it
    # in the Thread
    # returns:: TermVectorsReader
    def get_term_vectors_reader() 
      #return @xtv_reader ||= @tv_reader_orig.clone()
      tv_reader = Thread.current["#{self.object_id}-tv_reader"]
      if (tv_reader == nil) 
        tv_reader = @tv_reader_orig.clone()
        Thread.current["#{self.object_id}-tv_reader"] = tv_reader
      end
      return tv_reader
    end
    
    # Return a term frequency vector for the specified document and field. The
    # vector returned contains term numbers and frequencies for all terms in
    # the specified field of this document, if the field had storeTermVector
    # flag set.  If the flag was not set, the method returns nil.
    # raises:: IOException
    def get_term_vector(doc_number, field)
      # Check if this field is invalid or has no stored term vector
      fi = @field_infos[field]
      if fi.nil? or not fi.store_term_vector? or @tv_reader_orig.nil?
        return nil
      end
      
      term_vectors_reader = get_term_vectors_reader()
      if (term_vectors_reader == nil)
        return nil
      end
      return term_vectors_reader.get_field_tv(doc_number, field)
    end


    # Return an array of term frequency vectors for the specified document.
    # The array contains a vector for each vectorized field in the document.
    # Each vector vector contains term numbers and frequencies for all terms
    # in a given vectorized field.
    # If no such fields existed, the method returns nil.
    # raises:: IOException
    def get_term_vectors(doc_number)
      if @tv_reader_orig.nil?
        return nil
      end
      term_vectors_reader = get_term_vectors_reader()
      if (term_vectors_reader == nil)
        return nil
      end
      return term_vectors_reader.get_tv(doc_number)
    end

    def dir()
      return @directory
    end

    class Norm 
      attr_reader :is
      attr_writer :dirty
      attr_accessor :bytes

      def dirty?
        return @dirty
      end

      def initialize(is, number) 
        @is = is
        @number = number
      end

      def re_write(directory, segment, count, cfs_reader)
        # NOTE: norms are re-written in regular directory, not cfs
        out = directory.create_output(segment + ".tmp")
        begin 
          out.write_bytes(@bytes, count)
        ensure 
          out.close()
        end
        if(cfs_reader == nil)
            file_name = "#{segment}.f#{@number}"
        else
            # use a different file name if we have compound format
            file_name = "#{segment}.s#{@number}"
        end
        directory.rename(segment + ".tmp", file_name)
        @dirty = false
      end
    end
  end
end
