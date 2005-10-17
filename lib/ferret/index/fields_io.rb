require 'zlib'

include Ferret::Document

module Ferret
  module Index


    # Class responsible for access to stored document fields.
    # 
    # It uses &lt;segment&gt;.fdt and &lt;segment&gt;.fdx; files.
    class FieldsReader 
      attr_reader :size
      alias :length :size
      
      def initialize(d, segment, fi)
        @field_infos = fi

        @fields_stream = d.open_input(segment + ".fdt")
        @index_stream = d.open_input(segment + ".fdx")

        @size = (@index_stream.length() / 8).to_i
      end

      def close()
        @fields_stream.close()
        @index_stream.close()
      end


      def doc(n)
        @index_stream.seek(n * 8)
        position = @index_stream.read_long()
        @fields_stream.seek(position)

        doc = Document::Document.new
        @fields_stream.read_vint().times do
          field_number = @fields_stream.read_vint()
          fi = @field_infos[field_number]

          bits = @fields_stream.read_byte()
          
          compressed = (bits & FieldsWriter::FIELD_IS_COMPRESSED) != 0
          tokenize = (bits & FieldsWriter::FIELD_IS_TOKENIZED) != 0
          binary = (bits & FieldsWriter::FIELD_IS_BINARY) != 0
          
          if binary
            b = " " * @fields_stream.read_vint()
            @fields_stream.read_bytes(b, 0, b.length)
            if compressed
              doc << Field.new_binary_field(fi.name,
                                            uncompress(b),
                                            Field::Store::COMPRESS)
            else # No compression
              doc << Field.new_binary_field(fi.name, b, Field::Store::YES)
            end
          else 
            store = Field::Store::YES
            if fi.indexed? and tokenize
              index = Field::Index::TOKENIZED
            elsif fi.indexed? and not tokenize
              index = Field::Index::UNTOKENIZED
            else
              index = Field::Index::NO
            end
            data = nil
            if (compressed) 
              store = Field::Store::COMPRESS
              b = " " * @fields_stream.read_vint()
              @fields_stream.read_bytes(b, 0, b.length)
              data = uncompress(b)
            else
              data = @fields_stream.read_string()
            end
            stv =  Field::TermVector::NO
            if fi.store_term_vector? 
              if fi.store_positions? and fi.store_offsets?
                stv =  Field::TermVector::WITH_POSITIONS_OFFSETS 
              elsif fi.store_positions?
                stv =  Field::TermVector::WITH_POSITIONS
              elsif fi.store_offsets?
                stv =  Field::TermVector::WITH_OFFSETS 
              else
                stv =  Field::TermVector::YES 
              end
            end
            doc << Field.new(fi.name, data, store, index, stv)
          end
        end

        return doc
      end
      
			def uncompress(input)
				zstream = Zlib::Inflate.new
				buf = zstream.inflate(input)
				zstream.finish
				zstream.close
				buf
			end
    end


    class FieldsWriter

      FIELD_IS_TOKENIZED = 0X1
      FIELD_IS_BINARY = 0X2
      FIELD_IS_COMPRESSED = 0X4 

      def initialize(dir, segment, fi)
        @field_infos = fi
        @fields_stream = dir.create_output(segment + ".fdt")
        @index_stream = dir.create_output(segment + ".fdx")
      end

      def close()
        @fields_stream.close()
        @index_stream.close()
      end

      def add_document(doc)
        @index_stream.write_long(@fields_stream.pos)
        stored_count = 0
        doc.all_fields.each() { |field| stored_count += 1 if field.stored?() }
        @fields_stream.write_vint(stored_count)
        
        doc.all_fields.each() do |field|
          if (field.stored?())
            @fields_stream.write_vint(@field_infos.field_number(field.name))

            bits = 0
						bits |= FIELD_IS_TOKENIZED if field.tokenized?
						bits |= FIELD_IS_BINARY if field.binary?
						bits |= FIELD_IS_COMPRESSED if field.compressed?
            @fields_stream.write_byte(bits)
            
            data = nil
            if field.compressed?
              if field.binary? 
                data = compress(field.binary_value)
              else
                data = compress(field.string_value)
              end
              save_data(data)
            else
              if field.binary?
                save_data(field.binary_value)
              else
                @fields_stream.write_string(field.string_value)
              end
            end
          end
        end
      end
      alias :<< :add_document

      private
      
        def compress(input)
          zstream = Zlib::Deflate.new(Zlib::BEST_COMPRESSION)
          buf = zstream.deflate(input, Zlib::FINISH)
          zstream.close
          return buf
        end

        def save_data(data)
          len = data.length
          if data.is_a? Array
            data = data.pack("C*")
          end

          @fields_stream.write_vint(len)
          @fields_stream.write_bytes(data, len)
        end
    end
  end
end
