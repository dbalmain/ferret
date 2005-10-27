module Ferret
  module Index
    # Holds the info for one segment.
    #
    # ToDo: Does the dir really need to be stored here?
    class SegmentInfo
      attr_accessor :name, :doc_count, :directory

      def initialize(name, doc_count, dir)
        @name = name
        @doc_count = doc_count
        @directory = dir
      end

      def ==(o)
        (o.name == @name and o.doc_count == @doc_count)
      end
    end

    class SegmentInfos < Array
      # for compatability with Java Ferret files
      FORMAT = -1
      SEGMENT_FILENAME = "segments"
      TEMPORARY_SEGMENT_FILENAME = "segments.new"

      attr_reader :version    # counts how often the index has been modified
                              # by adding or deleting docs
      attr_accessor :counter  # used to name new segments??

      # Current version number from segments file.
      def SegmentInfos.read_current_version(directory)
        return 0 if not directory.exists?(SEGMENT_FILENAME)
        input = directory.open_input(SEGMENT_FILENAME)
        @format = 0
        @version = 0
        begin
          @format = input.read_int()
          if(@format < 0)
            if (@format < FORMAT) then raise "Unknown format version: " + @format end
            @version = input.read_long() # read version
          end
        ensure
          input.close()
        end
         
        if(@format < 0)
          return @version
        end

        # We cannot be sure about the format of the file.
        # Therefore we have to read the whole file and cannot simply
        # seek to the version entry.

        sis = SegmentInfos.new()
        sis.read(directory)
        return sis.version()
      end
      
      def initialize()
        @version = Time.now.to_i * 1000
        @counter = 0
      end

      def initialize_copy(o)
        super
        o.each_index {|i| self[i] = o[i].clone}
      end

      def read(directory)
        input = directory.open_input(SEGMENT_FILENAME)
        begin
          @format = input.read_int()
          if(@format < 0) # file contains explicit format info
            # check that it is a format we can understand
            if (@format < FORMAT) then raise "Unknown format version: " + @format end
            @version = input.read_long()
            @counter = input.read_int()
          else  # file is in old format without explicit format info
            @counter = @format
          end
          
          seg_count = input.read_int()
          seg_count.times do
            self << SegmentInfo.new(input.read_string(),
                                    input.read_int(),
                                    directory)
          end
         
          if(@format >= 0)   
            # in old format the version number may be at the end of the file
            if (input.pos() >= input.length())
              @version = 0 # old file format without version number
            else
              @version = input.read_long() # read version
            end
          end
        ensure
          input.close()
        end
      end

      def write(directory)
        output = directory.create_output(TEMPORARY_SEGMENT_FILENAME)
        begin
          output.write_int(FORMAT) # write FORMAT
          output.write_long(@version += 1) # every write changes the index
          output.write_int(@counter) # write counter
          output.write_int(size()) # write infos
          each() do |si|
            output.write_string(si.name)
            output.write_int(si.doc_count)
          end         
        
        ensure
          output.close()
        end

        # install new segment info
        directory.rename(TEMPORARY_SEGMENT_FILENAME, SEGMENT_FILENAME)
      end

      def to_s()
        str = "\nSegmentInfos: <"
        each() { |si| str << "#{si.name}:#{si.doc_count}," }
        str[-1] = ">"
        str
      end
    end
  end
end
