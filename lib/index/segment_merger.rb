include Ferret::Store
module Ferret::Index
  # The SegmentMerger class combines two or more Segments, represented by
  # an IndexReader#add, into a single Segment.  After adding the
  # appropriate readers, call the merge method to combine the segments.
  # 
  # If the compoundFile flag is set, then the segments will be merged
  # into a compound file.
  class SegmentMerger 

    # dir:: The Directory to merge the other segments into
    # name:: The name of the new segment
    def initialize(dir, name,
                  term_index_interval = IndexWriter::DEFAULT_TERM_INDEX_INTERVAL) 
      @directory = dir
      @segment = name
      @term_index_interval = term_index_interval
      @readers = []
      @field_infos = nil
      @freq_output = nil
      @prox_output = nil
      @term_infos_writer = nil
      @queue = nil
      @term_info = TermInfo.new()
      @skip_buffer = RAMDirectory::RAMIndexOutput.new(RAMDirectory::RAMFile.new(""))
    end

    # Add an IndexReader to the collection of readers that are to be merged
    # reader::
    def add(reader) 
      @readers << reader
    end

    # 
    # i:: The index of the reader to return
    # returns:: The ith reader to be merged
    def segment_reader(i) 
      return @readers[i]
    end

    # Merges the readers specified by the @link #addendmethod into the directory passed to the constructor
    # returns:: The number of documents that were merged
    # raises:: IOError
    def merge()
      value = merge_fields()
      merge_terms()
      merge_norms()
      merge_vectors() if @field_infos.has_vectors?
      return value
    end
    
    # close all IndexReaders that have been added.
    # Should not be called before merge().
    # raises:: IOError
    def close_readers()
      @readers.each { |reader| reader.close }
    end

    def create_compound_file(file_name)
           
      cfs_writer = CompoundFileWriter.new(@directory, file_name)

      files = Array.new(IndexFileNames::COMPOUND_EXTENSIONS.length + @field_infos.size)
      
      # Basic files
      IndexFileNames::COMPOUND_EXTENSIONS.times do |i|
        files << @segment + "." + IndexFileNames::COMPOUND_EXTENSIONS[i]
      end

      # Field norm files
      @field_infos.each_with_index do |fi, i|
        if (fi.indexed?) 
          files << @segment + ".f#{i}"
        end
      end

      # Vector files
      if @field_infos.has_vectors?
        IndexFileNames::VECTOR_EXTENSIONS.length.times do |i|
          files << @segment + "." + IndexFileNames::VECTOR_EXTENSIONS[i]
        end
      end

      # Now merge all added files
      files.each do |file|
        cfs_writer.add_file(file)
      end
      
      # Perform the merge
      cfs_writer.close
     
      return files
    end

    # 
    # returns:: The number of documents in all of the readers
    # raises:: IOError
    def merge_fields()
      @field_infos = FieldInfos.new()      # merge field names
      doc_count = 0
      @readers.each do |reader|
        @field_infos.add_fields(reader.get_field_names(IndexReader::FieldOption::TERM_VECTOR_WITH_POSITION_OFFSET), true, true, true, true)
        @field_infos.add_fields(reader.get_field_names(IndexReader::FieldOption::TERM_VECTOR_WITH_POSITION), true, true, true, false)
        @field_infos.add_fields(reader.get_field_names(IndexReader::FieldOption::TERM_VECTOR_WITH_OFFSET), true, true, false, true)
        @field_infos.add_fields(reader.get_field_names(IndexReader::FieldOption::TERM_VECTOR), true, true, false, false)
        @field_infos.add_fields(reader.get_field_names(IndexReader::FieldOption::INDEXED), true, false, false, false)
        @field_infos.add_fields(reader.get_field_names(IndexReader::FieldOption::UNINDEXED), false)
      end
      @field_infos.write_to_dir(@directory, @segment + ".fnm")

      # merge field values
      fields_writer = FieldsWriter.new(@directory, @segment, @field_infos)

      begin 
        @readers.each do |reader|
          max_doc = reader.max_doc()
          max_doc.times do |j|
            if not reader.deleted?(j) # skip deleted docs
              fields_writer.add_document(reader.get_document(j))
              doc_count += 1
            end
          end
        end
      ensure 
        fields_writer.close()
      end
      return doc_count
    end

    # Merge the TermVectors from each of the segments into the new one.
    # raises:: IOError
    def merge_vectors()
      term_vectors_writer = TermVectorsWriter.new(@directory, @segment, @field_infos)

      begin 
        @readers.each do |reader|
          max_doc = reader.max_doc()
          max_doc.times do |doc_num|
            # skip deleted docs
            next if (reader.deleted?(doc_num)) 
            term_vectors_writer.add_all_doc_vectors(reader.get_term_vectors(doc_num))
          end
        end
      ensure 
        term_vectors_writer.close()
      end
    end

    def merge_terms()
      begin 
        @freq_output = @directory.create_output(@segment + ".frq")
        @prox_output = @directory.create_output(@segment + ".prx")
        @term_infos_writer =
                TermInfosWriter.new(@directory, @segment, @field_infos,
                                    @term_index_interval)
        @skip_interval = @term_infos_writer.skip_interval
        @queue = SegmentMergeQueue.new(@readers.size())

        merge_term_infos()

      ensure 
        [@freq_output, @prox_output, @term_infos_writer, @queue].each do |obj|
          obj.close()
        end
      end
    end

    def merge_term_infos()
      base = 0
      @readers.each do |reader|
        term_enum = reader.terms()
        smi = SegmentMergeInfo.new(base, term_enum, reader)
        base += reader.num_docs()
        if (smi.next?)
          @queue.push(smi) # initialize @queue
        else
          smi.close()
        end
      end

      match = Array.new(@readers.size)

      while (@queue.size > 0) 
        match_size = 0     # pop matching terms
        match[match_size] = @queue.pop
        match_size += 1
        term = match[0].term
        top = @queue.top

        while top and term == top.term
          match[match_size] = @queue.pop
          match_size += 1
          top = @queue.top
        end

        merge_term_info(match, match_size)      # add new TermInfo

        while (match_size > 0) 
          match_size -= 1
          smi = match[match_size]
          if (smi.next?)
            @queue.push(smi) # restore queue
          else
            smi.close()      # done with a segment
          end
        end
      end
    end

    # Merge one term found in one or more segments. The array <code>smis</code>
    # contains segments that are positioned at the same term. <code>N</code>
    # is the number of cells in the array actually occupied.
    # 
    # smis:: array of segments
    # n:: number of cells in the array actually occupied
    def merge_term_info(smis, n)
           
      freq_pointer = @freq_output.pos
      prox_pointer = @prox_output.pos

      df = append_postings(smis, n)      # append posting data

      skip_pointer = write_skip()

      if (df > 0) 
        # add an enbegin to the dictionary with pointers to prox and freq files
        @term_info.set_values!(df, freq_pointer, prox_pointer, (skip_pointer - freq_pointer))
        @term_infos_writer.add(smis[0].term, @term_info)
      end
    end

    # Process postings from multiple segments all positioned on the
    # same term. Writes out merged entries into @freq_utput and
    # the @prox_output streams.
    # 
    # smis:: array of segments
    # n:: number of cells in the array actually occupied
    # returns:: number of documents across all segments where this term was found
    def append_postings(smis, n)
      last_doc = 0
      df = 0            # number of docs w/ term
      reset_skip()
      n.times do |i|
        smi = smis[i]
        postings = smi.postings
        base = smi.base
        doc_map = smi.doc_map
  
        postings.seek(smi.term_enum)
        while (postings.next?) 
          doc = postings.doc()
          doc = doc_map[doc] if (doc_map != nil) # work around deletions
          doc += base                            # convert to merged space

          if (doc < last_doc)
            raise "docs out of order curent doc = " + doc.to_s +
              " and previous doc = " + last_doc.to_s
          end

          df += 1

          if ((df % @skip_interval) == 0) 
            buffer_skip(last_doc)
          end

          doc_code = (doc - last_doc) << 1    # use low bit to flag freq=1
          last_doc = doc

          freq = postings.freq
          if (freq == 1) 
            @freq_output.write_vint(doc_code | 1) # write doc & freq=1
          else 
            @freq_output.write_vint(doc_code)     # write doc
            @freq_output.write_vint(freq)         # write frequency in doc
          end

          last_position = 0        # write position deltas
          freq.times do |j|
            position = postings.next_position()
            @prox_output.write_vint(position - last_position)
            last_position = position
          end
        end
      end
      return df
    end

    def reset_skip() 
      @skip_buffer.reset()
      @last_skip_doc = 0
      @last_skip_freq_pointer = @freq_output.pos
      @last_skip_prox_pointer = @prox_output.pos
    end

    def buffer_skip(doc)
      freq_pointer = @freq_output.pos
      prox_pointer = @prox_output.pos

      @skip_buffer.write_vint(doc - @last_skip_doc)
      @skip_buffer.write_vint(freq_pointer - @last_skip_freq_pointer)
      @skip_buffer.write_vint(prox_pointer - @last_skip_prox_pointer)

      @last_skip_doc = doc
      @last_skip_freq_pointer = freq_pointer
      @last_skip_prox_pointer = prox_pointer
    end

    def write_skip()
      skip_pointer = @freq_output.pos
      @skip_buffer.write_to(@freq_output)
      return skip_pointer
    end

    def merge_norms()
      @field_infos.each_with_index do |fi, i|
        if (fi.indexed?) 
          output = @directory.create_output(@segment + ".f" + i.to_s)
          begin 
            @readers.each do |reader|
              max_doc = reader.max_doc()
              input = "0" * max_doc
              reader.set_norms(fi.name, input, 0)
              max_doc.times do |k|
                if not reader.deleted?(k) 
                  output.write_byte(input[k])
                end
              end
            end
          ensure 
            output.close()
          end
        end
      end
    end
  end
end
