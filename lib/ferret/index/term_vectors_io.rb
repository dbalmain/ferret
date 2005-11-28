module Ferret::Index
  # Writer works by opening a document and then opening the fields within
  # the document and then writing out the vectors for each field.
  # 
  # Rough usage:
  # 
  #    for each document
  #    
  #      writer.open_document()
  #      for each field on the document
  #      
  #        writer.open_field(field)
  #        for all of the @terms
  #          
  #          writer.add_term(...)
  #        end
  #        writer.close_field
  #      end
  #      writer.close_document()    
  #    end
  # 
  # 
  class TermVectorsWriter 
    STORE_POSITIONS_WITH_TERMVECTOR = 0x1
    STORE_OFFSET_WITH_TERMVECTOR = 0x2
    
    FORMAT_VERSION = 2

    # The size in bytes that the FORMAT_VERSION will take up at the beginning
    # of each file 
    FORMAT_SIZE = 4
    
    TVX_EXTENSION = ".tvx"
    TVD_EXTENSION = ".tvd"
    TVF_EXTENSION = ".tvf"

    def initialize(directory, segment, field_infos)
      @current_field = nil
      @current_doc_pointer = -1
     
      # Open files for TermVector storage
      @tvx = directory.create_output(segment + TVX_EXTENSION)
      @tvx.write_int(FORMAT_VERSION)
      @tvd = directory.create_output(segment + TVD_EXTENSION)
      @tvd.write_int(FORMAT_VERSION)
      @tvf = directory.create_output(segment + TVF_EXTENSION)
      @tvf.write_int(FORMAT_VERSION)

      @field_infos = field_infos
      @fields = []
      @terms = []
    end


    def open_document()
      close_document()
      @current_doc_pointer = @tvd.pos()
    end


    def close_document()
           
      if (document_open?()) 
        close_field()
        write_doc()
        @fields.clear()
        @current_doc_pointer = -1
      end
    end


    def document_open?() 
      return @current_doc_pointer != -1
    end


    # Start processing a field. This can be followed by a number of calls to
    # add_term, and a final call to close_field to indicate the end of
    # processing of this field. If a field was previously open, it is closed
    # automatically.
    def open_field(field)
      field_info = @field_infos[field]
      create_field(field_info.number,
                   field_info.store_positions?,
                   field_info.store_offsets?)
    end
    
    # Finished processing current field. This should be followed by a call
    # to open_field before future calls to add_term.
    def close_field()
      if field_open?
        #puts("close_field()")

        # save field and @terms
        write_field()
        @fields << @current_field
        @terms.clear()
        @current_field = nil
      end
    end

    # Return true if a field is currently open. 
    def field_open?() 
      return @current_field != nil
    end

    # Add term to the field's term vector. Field must already be open.
    #
    # Terms should be added in increasing order of @terms, one call per
    # unique termNum. ProxPointer is a pointer into the TermPosition file
    # (prx). Freq is the number of times this term appears in this field, in
    # this document.  raises:: IllegalStateException if document or field is
    # not open
    def add_term(term_text, freq, positions = nil, offsets = nil)
      if not document_open?
        raise IllegalStateError, "Cannot add terms when document is not open"
      end
      if not field_open?
        raise IllegalStateError, "Cannot add terms when field is not open"
      end
      
      add_term_internal(term_text, freq, positions, offsets)
    end

    def add_term_internal(term_text, freq, positions, offsets) 
      @terms << TVTerm.new(term_text, freq, positions, offsets)
    end

    # Add a complete document specified by all its term vectors. If document has no
    # term vectors, add value for @tvx.
    # 
    # vectors:: The documents to have their term vectors added
    # raises:: IOException
    def add_all_doc_vectors(vectors)
       
      open_document()

      if vectors != nil 
        vectors.each do |vector|
          store_positions = (vector.size > 0 and vector.positions != nil)
          store_offsets = (vector.size > 0 and vector.offsets != nil)

          create_field(@field_infos.field_number(vector.field),
                       store_positions, store_offsets)

          vector.size.times do |j|
            add_term_internal(vector.terms[j],
                              vector.term_frequencies[j],
                              store_positions ? vector.positions[j] : nil,
                              store_offsets ? vector.offsets[j] : nil)
          end
          close_field()
        end
      end
      close_document()
    end
    
    # Close all streams. 
    def close()
      begin 
        close_document()
      ensure 
        # make an effort to close all streams we can but remember and re-raise
        # the last exception encountered in this process
        keep = nil
        [@tvx, @tvd, @tvf].compact.each do |os|
          begin 
            os.close()
          rescue IOError => e
            keep = e
          end
        end
        raise keep if (keep != nil) 
      end
    end

    class TVField 
      attr_accessor :number, :tvf_pointer, :store_positions, :store_offsets
      def initialize(number, store_pos, store_off) 
        @tvf_pointer = 0
        @number = number
        @store_positions = store_pos
        @store_offsets = store_off
      end
    end

    class TVTerm 
      attr_accessor :term_text, :freq, :positions, :offsets
      
      def initialize(term_text=nil, freq=nil, positions=nil, offsets=nil)
        @term_text = term_text
        @freq = freq
        @positions = positions
        @offsets = offsets
      end
    end

    private

      def write_field()
        ($cnt||=0)
        puts $cnt += 1
        # remember where this field is written
        @current_field.tvf_pointer = @tvf.pos
        
        size = @terms.size
        @tvf.write_vint(size)
        
        store_positions = @current_field.store_positions
        store_offsets = @current_field.store_offsets
        bits = 0x0
        if (store_positions) 
          bits |= STORE_POSITIONS_WITH_TERMVECTOR
        end
        if (store_offsets) 
          bits |= STORE_OFFSET_WITH_TERMVECTOR
        end
        @tvf.write_byte(bits)
        
        last_term_text = ""
        @terms.each do |term|
          start = Ferret::Utils::StringHelper.string_difference(last_term_text,
                                                                term.term_text)
          length = term.term_text.length() - start
          @tvf.write_vint(start)       # write shared prefix length
          @tvf.write_vint(length)      # write delta length
          @tvf.write_chars(term.term_text, start, length)  # write delta chars
          @tvf.write_vint(term.freq)
          last_term_text = term.term_text
          
          if (store_positions)
            if (term.positions == nil)
              raise IllegalStateError, "Trying to write positions that are nil!"
            end
            
            # use delta encoding for positions
            position = 0
            term.freq.times do |j|
              @tvf.write_vint(term.positions[j] - position)
              position = term.positions[j]
            end
          end
          
          if (store_offsets)
            if(term.offsets == nil)
              raise IllegalStateError, "Trying to write offsets that are nil!"
            end
            
            # use delta encoding for offsets
            position = 0
            term.freq.times do |j|
              @tvf.write_vint(term.offsets[j].start_offset - position)
              #Save the diff between the two.
              @tvf.write_vint(term.offsets[j].end_offset -
                              term.offsets[j].start_offset)
              position = term.offsets[j].end_offset()
            end
          end
        end
      end

      def write_doc()
        if field_open?
          raise IllegalStateError, "Field is still open while writing document"
        end
        #puts("Writing doc pointer: " + @current_doc_pointer)
        # write document index record
        @tvx.write_long(@current_doc_pointer)

        # write document data record
        size = @fields.size

        # write the number of @fields
        @tvd.write_vint(size)

        # write field numbers
        @fields.each { |field| @tvd.write_vint(field.number) }

        # write field pointers
        last_field_pointer = 0
        @fields.each do |field|
          @tvd.write_vlong(field.tvf_pointer - last_field_pointer)
          last_field_pointer = field.tvf_pointer
        end
        #puts("After writing doc pointer: " + @tvx.pos())
      end

      def create_field(field_number, store_position, store_offset)
        if not document_open? 
          raise IllegalStateError, "Cannot open field when no document is open."
        end
        close_field()
        @current_field = TVField.new(field_number, store_position, store_offset)
      end
  end

  class TermVectorsReader
    attr_reader :size

    # accessors for clone method
    attr_accessor :tvx, :tvd, :tvf
    protected :tvx, :tvx=, :tvd, :tvd=, :tvf, :tvf=
    

    def initialize(d, segment, field_infos)
     
      if (d.exists?(segment + TermVectorsWriter::TVX_EXTENSION)) 
        @tvx = d.open_input(segment + TermVectorsWriter::TVX_EXTENSION)
        check_valid_format(@tvx)
        @tvd = d.open_input(segment + TermVectorsWriter::TVD_EXTENSION)
        @tvd_format = check_valid_format(@tvd)
        @tvf = d.open_input(segment + TermVectorsWriter::TVF_EXTENSION)
        @tvf_format = check_valid_format(@tvf)
        @size = @tvx.length / 8
      else
        @tvx = nil
        @tvd = nil
        @tvf = nil
      end

      @field_infos = field_infos
    end
    
    def close()
      # make an effort to close all streams we can but remember and re-raise
      # the last exception encountered in this process
      keep = nil
      [@tvx, @tvd, @tvf].compact.each do |os|
        begin 
          os.close()
        rescue IOError => e
          keep = e
        end
      end
      raise keep if (keep != nil) 
    end

    # Retrieve the term vector for the given document and field
    # doc_num:: The document number to retrieve the vector for
    # field:: The field within the document to retrieve
    # returns:: The TermFreqVector for the document and field or nil if there
    #   is no termVector for this field.
    # raises:: IOException if there is an error reading the term vector files
    def get_field_tv(doc_num, field)
      # Check if no term vectors are available for this segment at all
      field_number = @field_infos.field_number(field)
      result = nil
      if (@tvx != nil) 
        #We need to account for the FORMAT_SIZE at when seeking in the @tvx
        #We don't need to do this in other seeks because we already have the
        # file pointer
        #that was written in another file
        @tvx.seek((doc_num * 8) + TermVectorsWriter::FORMAT_SIZE)
        #puts("TVX Pointer: " + @tvx.pos())
        position = @tvx.read_long()

        @tvd.seek(position)
        field_count = @tvd.read_vint()
        #puts("Num Fields: " + field_count)
        # There are only a few fields per document. We opt for a full scan
        # rather then requiring that they be ordered. We need to read through
        # all of the fields anyway to get to the tvf pointers.
        number = 0
        found = -1
        field_count.times do |i|
          if @tvd_format == TermVectorsWriter::FORMAT_VERSION
            number = @tvd.read_vint()
          else
            number += @tvd.read_vint()
          end
          if (number == field_number)
            found = i
          end
        end

        # This field, although valid in the segment, was not found in this
        # document
        if (found != -1) 
          # Compute position in the @tvf file
          position = 0
          (found + 1).times do 
            position += @tvd.read_vlong()
          end

          result = read_term_vector(field, position)
        end
      end
      return result
    end

    # Return all term vectors stored for this document or nil if it could
    # not be read in.
    # 
    # doc_num:: The document number to retrieve the vector for
    # returns:: All term frequency vectors
    # raises:: IOException if there is an error reading the term vector files 
    def get_tv(doc_num)
      result = nil
      # Check if no term vectors are available for this segment at all
      if (@tvx != nil) 
        #We need to offset by
        @tvx.seek((doc_num * 8) + TermVectorsWriter::FORMAT_SIZE)
        position = @tvx.read_long()

        @tvd.seek(position)
        field_count = @tvd.read_vint()

        # No fields are vectorized for this document
        if (field_count != 0) 
          number = 0
          fields = Array.new(field_count)
          
          field_count.times do |i|
            if @tvd_format == TermVectorsWriter::FORMAT_VERSION
              number = @tvd.read_vint()
            else
              number += @tvd.read_vint()
            end

            fields[i] = @field_infos[number].name
          end

          # Compute position in the @tvf file
          position = 0
          tvf_pointers = Array.new(field_count)
          field_count.times do |i|
            position += @tvd.read_vlong()
            tvf_pointers[i] = position
          end

          result = read_term_vectors(fields, tvf_pointers)
        end
      end
      return result
    end

    def clone() 
      
      if (@tvx == nil or @tvd == nil or @tvf == nil)
        return nil
      end
      
      clone = self
      clone.tvx = @tvx.clone()
      clone.tvd = @tvd.clone()
      clone.tvf = @tvf.clone()
      
      return clone
    end

    private 

      def read_term_vectors(fields, tvf_pointers)
             
        res = Array.new(fields.length)
        fields.length.times do |i|
          res[i] = read_term_vector(fields[i], tvf_pointers[i])
        end
        return res
      end

      # field:: The field to read in
      # tvf_pointer:: The pointer within the @tvf file where we should start reading
      # returns:: The TermVector located at that position
      # raises:: IOException
      def read_term_vector(field, tvf_pointer)
        # Now read the data from specified position
        # We don't need to offset by the FORMAT here since the pointer
        # already includes the offset
        @tvf.seek(tvf_pointer)

        num_terms = @tvf.read_vint()
        # If no terms - return a constant empty termvector. However, this should
        # never occur!
        if (num_terms == 0) 
          return SegmentTermVector.new(field, nil, nil)
        end
        
        
        if(@tvf_format == TermVectorsWriter::FORMAT_VERSION)
          bits = @tvf.read_byte()
          store_positions = (bits & TermVectorsWriter::STORE_POSITIONS_WITH_TERMVECTOR) != 0
          store_offsets = (bits & TermVectorsWriter::STORE_OFFSET_WITH_TERMVECTOR) != 0
        else
          @tvf.read_vint()
          store_positions = false
          store_offsets = false
        end

        terms = Array.new(num_terms)
        term_freqs = Array.new(num_terms)
        
        #  we may not need these, but declare them
        positions = nil
        offsets = nil
        if(store_positions)
          positions = Array.new(num_terms)
        end
        if(store_offsets)
          offsets = Array.new(num_terms)
        end
        
        start = 0
        delta_length = 0
        total_length = 0
        buffer = "" 
        previous_buffer = ""
        
        num_terms.times do |i|
          start = @tvf.read_vint()
          delta_length = @tvf.read_vint()
          total_length = start + delta_length
          @tvf.read_chars(buffer, start, delta_length)
          terms[i] = buffer[0, total_length].to_s
          previous_string = terms[i]
          freq = @tvf.read_vint()
          term_freqs[i] = freq
          
          if (store_positions) #read in the positions
            pos = Array.new(freq)
            positions[i] = pos
            prev_position = 0
            freq.times do |j|
              pos[j] = prev_position + @tvf.read_vint()
              prev_position = pos[j]
            end
          end
          
          if (store_offsets) 
            offs = Array.new(freq)
            offsets[i] = offs
            prev_offset = 0
            freq.times do |j|
              start_offset = prev_offset + @tvf.read_vint()
              end_offset = start_offset + @tvf.read_vint()
              offs[j] = TermVectorOffsetInfo.new(start_offset, end_offset)
              prev_offset = end_offset
            end
          end
        end
        
        SegmentTermVector.new(field, terms, term_freqs, positions, offsets)
      end

      def check_valid_format(istream)
        format = istream.read_int()
        if (format > TermVectorsWriter::FORMAT_VERSION)
          raise IOError, "Incompatible format version: #{format} expected #{TermVectorsWriter::FORMAT_VERSION} or less"
        end
        return format
      end

  end
end
