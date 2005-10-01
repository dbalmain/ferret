require 'search/similarity'
module Ferret::Index
  include Ferret::Search
  class DocumentWriter 
    # If non-nil, a message will be printed to this if max_field_length is reached.
    attr_writer :info_stream

    # directory:: The directory to write the document information to
    # analyzer:: The analyzer to use for the document
    # similarity:: The Similarity function
    #     <writer.similarity>
    # max_field_length:: The maximum number of tokens a field may have
    #     <writer.max_field_length>
    # term_index_interval:: The interval of terms in the index
    #     <writer.max_field_length>
    def initialize(directory,
                   analyzer,
                   similarity,
                   max_field_length,
                   term_index_interval = IndexWriter::DEFAULT_TERM_INDEX_INTERVAL) 
      @directory = directory
      @analyzer = analyzer
      @similarity = similarity
      @max_field_length = max_field_length
      @term_index_interval = term_index_interval

      # Keys are Terms, values are Postings.
      # Used to buffer a document before it is written to the index.
      @posting_table = {}

      @term_buffer = Term.new("", "")
    end

    def add_document(segment, doc)
           
      # write field names
      @field_infos = FieldInfos.new()
      @field_infos << doc
      @field_infos.write_to_dir(@directory, segment + ".fnm")

      # write field values
      fields_writer = FieldsWriter.new(@directory, segment, @field_infos)
      begin 
        fields_writer.add_document(doc)
      ensure 
        fields_writer.close()
      end

      # invert doc into posting_table
      @posting_table.clear();        # clear posting_table
      arr_size = @field_infos.size
      @field_lengths = Array.new(arr_size, 0)    # init field_lengths
      @field_positions = Array.new(arr_size, 0)  # init field_positions
      @field_offsets = Array.new(arr_size, 0)    # init field_offsets
      @field_boosts = Array.new(arr_size, doc.boost)     # init field_boosts

      invert_document(doc)

      # sort posting_table into an array
      postings = sort_posting_table()

  #    for (int i = 0; i < postings.length; i += 1) 
  #      Posting posting = postings[i]
  #      print(posting.term)
  #      print(" freq=" + posting.freq)
  #      print(" pos=")
  #      print(posting.positions[0])
  #      for (int j = 1; j < posting.freq; j += 1)
  #        print("," + posting.positions[j])
  #      puts("")
  #    end

      # write postings
      write_postings(postings, segment)

      # write norms of indexed fields
      write_norms(segment)

    end

    private

      # Tokenizes the fields of a document into Postings.
      def invert_document(doc)
             
        fields = doc.all_fields
        fields.each do |field|
          field_name = field.name
          field_number = @field_infos.field_number(field_name)

          length = @field_lengths[field_number]     # length of field
          position = @field_positions[field_number] # position in field
          offset = @field_offsets[field_number]     # offset field

          if field.indexed? 
            if not field.tokenized? # un-tokenized field
              string_value = field.string_value
              if field.store_offset?
                add_position(field_name,
                             string_value,
                             position,
                             TermVectorOffsetInfo.new(offset,
                                      offset + string_value.length))
                position += 1
              else
                add_position(field_name, string_value, position, nil)
                position += 1
              end
              offset += string_value.length()
              length += 1
            else 
            
              reader = field.reader_value()

              # Tokenize field and add to posting_table
              stream = @analyzer.token_stream(field_name, reader)
              begin 
                last_token = nil
                while token = stream.next
                  position += (token.position_increment - 1)
                  
                  if(field.store_offset?())
                    add_position(field_name,
                                 token.term_text(),
                                 position,
                                 TermVectorOffsetInfo.new(
                                   offset + token.start_offset(),
                                   offset + token.end_offset()))
                    position += 1
                  else
                    add_position(field_name, token.term_text(), position, nil)
                    position += 1
                  end
                  
                  last_token = token
                  length += 1
                  if (length > @max_field_length) 
                    if @info_stream
                      @info_stream.puts("max_field_length " + @max_field_length.to_s + " reached, ignoring following tokens")
                    end
                    break
                  end
                end
                
                if(last_token != nil)
                  offset += last_token.end_offset() + 1
                end
                
              ensure 
                stream.close()
              end
            end

            @field_lengths[field_number] = length    # save field length
            @field_positions[field_number] = position    # save field position
            @field_boosts[field_number] *= field.boost
            @field_offsets[field_number] = offset
          end
        end
      end


      def add_position(field, text, position, tv_offset_info) 
        @term_buffer.set!(field, text)
        #puts("Offset: " + tv_offset_info)
        posting = @posting_table[@term_buffer]
        if (posting != nil) # word seen before
          freq = posting.freq
          posting.positions[freq] = position     # add new position
          posting.offsets[freq] = tv_offset_info # add new position

          if (tv_offset_info != nil) 
            posting.offsets[freq] = tv_offset_info
          end
          posting.freq = freq + 1        # update frequency
        else # word not seen before
          term = Term.new(field, text)
          @posting_table[term] = Posting.new(term, position, tv_offset_info)
        end
      end

      def sort_posting_table() 
        # copy @posting_table into an array
        return @posting_table.values.sort { |x,y| x.term <=> y.term }
      end

      def write_postings(postings, segment)
             
        freq = nil
        prox = nil
        tis_writer = nil
        tv_writer = nil
        begin 
          #open files for inverse index storage
          freq = @directory.create_output(segment + ".frq")
          prox = @directory.create_output(segment + ".prx")
          tis_writer = TermInfosWriter.new(@directory, segment, @field_infos,
                                    @term_index_interval)
          ti = TermInfo.new()
          current_field = nil

          postings.each do |posting|
            # add an entry to the dictionary with pointers to prox and freq files
            ti.set_values!(1, freq.pos(), prox.pos(), -1)
            tis_writer.add(posting.term, ti)

            # add an entry to the freq file
            posting_freq = posting.freq
            if (posting_freq == 1)          # optimize freq=1
              freq.write_vint(1)            # set low bit of doc num.
            else 
              freq.write_vint(0)            # the document number
              freq.write_vint(posting_freq) # frequency in doc
            end

            last_position = 0               # write positions
            posting.positions.each do |position|
              prox.write_vint(position - last_position)
              last_position = position
            end
            # check to see if we switched to a new field
            term_field = posting.term.field_name
            if (current_field != term_field) 
              # changing field - see if there is something to save
              current_field = term_field
              fi = @field_infos[current_field]
              if (fi.store_term_vector?) 
                if tv_writer.nil? 
                  tv_writer = TermVectorsWriter.new(@directory, segment, @field_infos)
                  tv_writer.open_document()
                end
                tv_writer.open_field(current_field)

              elsif not tv_writer.nil?
                tv_writer.close_field()
              end
            end
            if not tv_writer.nil? and tv_writer.field_open?
                tv_writer.add_term(posting.term.text, posting_freq, posting.positions, posting.offsets)
            end
          end
          if not tv_writer.nil?
            tv_writer.close_document()
          end
        ensure 
          # make an effort to close all streams we can but remember and re-raise
          # the last exception encountered in this process
          keep = nil
          [freq, prox, tis_writer, tv_writer].compact.each do |obj|
            begin
              obj.close
            rescue IOError => e
              keep = e
            end
          end
          raise keep if not keep.nil?
        end
      end

      def write_norms(segment)
        @field_infos.each_with_index do |fi, i|
          if fi.indexed?
            norm = @field_boosts[i] * @similarity.length_norm(fi.name, @field_lengths[i])
            puts "norm for #{segment}:#{fi.name} = #{norm}, field_lengths = #{@field_lengths[i]}"
            norms = @directory.create_output(segment + ".f" + i.to_s)
            begin 
              norms.write_byte(Similarity.encode_norm(norm))
            ensure 
              norms.close()
            end
          end
        end
      end
      
  end

  class Posting # info about a Term in a doc
    attr_accessor :term, :freq, :positions, :offsets

    def initialize(t, position, offset) 
      @term = t
      @freq = 1
      @positions = [position]
      @offsets = [offset]
    end
  end
end
