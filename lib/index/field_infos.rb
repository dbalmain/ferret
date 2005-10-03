module Ferret
  module Index
    # Access to the Field Info file that describes document fields and whether or
    # not they are indexed. Each segment has a separate Field Info file. Objects
    # of this class are thread-safe for multiple readers, but only one thread can
    # be adding documents at a time, with no other reader or writer threads
    # accessing this object.
    class FieldInfos

      NOT_A_FIELD = 0xffffffff # -1 in java int

      # Construct a FieldInfos object using the directory and the name of the file
      # InputStream
      #
      # dir:: The directory to open the InputStream from
      # name:: The name of the file to open the InputStream from in the Directory
      def initialize(dir = nil, name = nil)
        @fi_array = []
        @fi_hash = {}
        if dir and dir.exists?(name)
          input = dir.open_input(name)
          begin
            read(input)
          ensure
            input.close()
          end
        end
      end

      # Returns the number of fields that have been added to this field infos
      # object.
      def size
        return @fi_array.size
      end

      # Automatically adds all of the fields from the document if they haven't
      # been added already. Or it will update the values.
      def add_doc_fields(doc)
        doc.all_fields.each do |field|
          add(field.name,
              field.indexed?,
              field.store_term_vector?,
              field.store_positions?,
              field.store_offsets?)
        end
      end
      alias :<< :add_doc_fields
      
      # Calls the 5 param add method to add all the names in the collection
      def add_fields(names, 
                    indexed = true,
                    store_term_vector = false,
                    store_position = false,
                    store_offset = false)
        names.each do |name|
          add(name, indexed, store_term_vector, store_position, store_offset)
        end
      end

      # If the field is not yet known, adds it. If it is known, checks to make
      # sure that the indexed flag is the same as was given previously for this
      # field. If not - marks it as being indexed.  Same goes for the TermVector
      # parameters.
      # 
      # name:: The name of the field
      # indexed:: true if the field is indexed
      # store_term_vector:: true if the term vector should be stored
      # store_position:: true if the positions should be stored
      # store_offset:: true if the offsets should be stored
      def add(name,
              indexed = true,
              store_term_vector = false,
              store_position = false,
              store_offset = false)
        fi = @fi_hash[name]
        if (fi == nil)
          fi = add_internal(name, indexed, store_term_vector, store_position, store_offset)
        else
          if (fi.indexed? != indexed)
            fi.indexed = true             # once indexed, always index
          end
          if (fi.store_term_vector? != store_term_vector)
            fi.store_term_vector = true   # once vector, always vector
          end
          if (fi.store_positions? != store_position)
            fi.store_position = true # once vector, always vector
          end
          if (fi.store_offsets? != store_offset)
            fi.store_offset = true   # once vector, always vector
          end
        end
        return fi
      end

      # Returns the number of the field that goes by the field name that is
      # passed. If there is no field of this name then -1 is returned
      def field_number(field_name)
        fi = @fi_hash[field_name]
        return fi ? fi.number : NOT_A_FIELD
      end

      # Retrieve the field_info object by either field number or field name.
      def [](index)
        if index.is_a? Integer
          if index == NOT_A_FIELD || index < 0 # < 0 is for C extensions
            return FieldInfo.new("", false, NOT_A_FIELD, false)
          end
          return @fi_array[index]
        else
          return @fi_hash[index]
        end
      end

      def field_name(index)
        if index == NOT_A_FIELD || index < 0 # < 0 is for C extensions
          return ""
        end
        return self[index].name
      end

      # Iterate through the field_info objects
      def each()
        @fi_array.each() {|fi| yield(fi) }
      end

      # Iterate through the field_info objects including the index
      def each_with_index()
        @fi_array.each_with_index() {|fi, i| yield(fi, i) }
      end

      # Get the number of field_infos in this object.
      #
      # NOTE: There is a default empty field always added at the start. This
      # may later be used to set the default values for a field.
      def size()
        return @fi_array.size()
      end

      # Return true if any of the fields have store_term_vector? set to true
      def has_vectors?()
        @fi_array.each() { |fi| return true if fi.store_term_vector? }
        return false
      end

      # Write the field_infos to a file specified by name in dir.
      #
      # dir:: the directory to write the fieldinfos to
      # name:: the name of the file to write to.
      def write_to_dir(dir, name)
        output = dir.create_output(name)
        begin
          write(output)
        ensure
          output.close()
        end
      end

      protected

        # Write the field_infos to the output file
        #
        # output:: the file to write to
        def write(output)
          output.write_vint(size())
          @fi_array.each() do |fi|
            output.write_string(fi.name)
            output.write_byte(get_field_info_byte(fi))
          end
        end

        # Read the field_infos object from the input file
        #
        # input:: the input file to read from
        def read(input)
          size = input.read_vint()#read in the size
          size.times do |i|
            name = input.read_string()
            bits = input.read_byte()
            indexed = (bits & IS_INDEXED) != 0
            store_term_vector = (bits & STORE_TERM_VECTOR) != 0
            store_position = (bits & STORE_POSITION) != 0
            store_offset = (bits & STORE_OFFSET) != 0
            add_internal(name, indexed, store_term_vector, store_position, store_offset)
          end
        end

      private
        IS_INDEXED = 0x1;
        STORE_TERM_VECTOR = 0x2;
        STORE_POSITION = 0x4;
        STORE_OFFSET = 0x8;

        def add_internal(name, indexed, store_term_vector,
                         store_position = false,
                         store_offset = false)
          fi = FieldInfo.new(name, indexed,
                             @fi_array.size(),
                             store_term_vector,
                             store_position,
                             store_offset)
          @fi_array << fi
          @fi_hash[name] = fi
          return fi
        end

        def get_field_info_byte(fi)
          bits = 0x0
          if (fi.indexed?)
            bits |= IS_INDEXED
          end
          if (fi.store_term_vector?)
            bits |= STORE_TERM_VECTOR
          end
          if (fi.store_positions?)
            bits |= STORE_POSITION
          end
          if (fi.store_offsets?)
            bits |= STORE_OFFSET
          end
          return bits
        end
    end

    class FieldInfo
      attr_accessor :name, :number
      attr_writer :indexed, :store_term_vector, :store_offset, :store_position

      def indexed?()
        return @indexed
      end

      def store_term_vector?()
        return @store_term_vector
      end

      def store_offsets?()
        return @store_offset
      end
      def store_positions?()
        return @store_position
      end

      def set!(indexed, store_term_vector, store_position, store_offset)
        @indexed = indexed
        @store_term_vector = store_term_vector
        @store_position = store_position
        @store_offset = store_offset
      end

      def initialize(name, indexed, number, store_term_vector,
                     store_position = false,
                     store_offset = false)
        @name = name
        @number = number
        set!(indexed, store_term_vector, store_position, store_offset)
      end
    end
  end
end
