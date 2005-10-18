require 'monitor'

module Ferret::Index

  # Class for accessing a compound stream.
  # This class implements a directory, but is limited to only read operations.
  # Directory methods that would normally modify data raise.
  class CompoundFileReader < Ferret::Store::Directory

    include MonitorMixin

    attr_reader :directory, :file_name

    # Creates a Compound File Reader which contains a single file and has
    # pointers to the individual files within. When it is initialized, the 
    # compound file is set and the header is read so that it is ready to read
    # the individual files within.
    def initialize(dir, name)

      super()

      @directory = dir
      @file_name = name
      @entries = {}

      success = false

      begin
        @stream = dir.open_input(name)

        # read the directory and init files
        count = @stream.read_vint()
        entry = nil
        count.times() do 
          offset = @stream.read_long()
          id = @stream.read_string()

          if (entry != nil)
            # set length of the previous entry
            entry.length = offset - entry.offset
          end

          entry = FileEntry.new(offset)
          @entries[id] = entry
        end

        # set the length of the final entry
        if (entry != nil)
          entry.length = @stream.length() - entry.offset
        end

        success = true

      ensure

        if not success and (@stream != nil)
          begin
            @stream.close()
          rescue IOError
          end
        end
      end
    end

    def close()
      synchronize do
        if (@stream == nil): raise(IOError, "Already closed") end

        @entries.clear()
        @stream.close()
        @stream = nil
      end
    end

    def open_input(id)
      synchronize do
        if (@stream == nil)
          raise(IOError, "Stream closed")
        end

        entry = @entries[id]
        if (entry == nil)
          raise(IOError, "No sub-file with id " + id + " found")
        end
        return CSIndexInput.new(@stream, entry.offset, entry.length)
      end
    end

    # Returns an array of strings, one for each file in the directory.
    def list()
      return @entries.keys()
    end

    # Returns true iff a file with the given name exists.
    def file_exists(name)
      return @entries.key?(name)
    end

    # Returns the time the named file was last modified.
    def modified(name)
      return @directory.modified(@file_name)
    end

    # Set the modified time of an existing file to now.
    def touch(name)
      @directory.touch(@file_name)
    end

    # Not implemented
    def delete(name) raise(UnsupportedOperationError) end

    # Not implemented
    def rename(from, to) raise(UnsupportedOperationError) end

    # Returns the length of a file in the directory.
    def file_length(name)
      e = @entries[name]
      if (e == nil): raise(IOError, "File " + name + " does not exist") end
      return e.length
    end

    # Not implemented
    def create_output(name) raise(UnsupportedOperationError) end

    # Not implemented
    def make_lock(name) raise(UnsupportedOperationError) end

    # Implementation of an IndexInput that reads from a portion of the
    # compound file.
    class CSIndexInput < Ferret::Store::BufferedIndexInput
      attr_reader :length

      def initialize(base, file_offset, length)
        super()
        @base = base
        @base.extend(MonitorMixin)
        @file_offset = file_offset
        @length = length
      end

      # Closes the stream to further operations.
      def close() end

      private
        # Expert: implements buffer refill.  Reads bytes from the current
        # position in the input.
        #
        # b:: the array to read bytes into
        # offset:: the offset in the array to start storing bytes
        # len:: the number of bytes to read
        def read_internal(b, offset, len)
          @base.synchronize() do
            start = pos()
            if(start + len > @length): raise(EOFError, "read past EOF") end
            @base.seek(@file_offset + start)
            @base.read_bytes(b, offset, len)
          end
        end

        # Expert: implements seek.  Sets current position in @file, where
        # the next {@link #read_internal(byte[],int,int)} will occur.
        def seek_internal(pos) end
    end

    private
      # Base info
      class FileEntry
        attr_accessor :offset, :length
        def initialize(offset)
          @offset = offset
        end
      end

  end

  # Combines multiple files into a single compound file.
  # The file format:
  #
  #   * VInt fileCount
  #   * {Directory} fileCount entries with the following structure:
  #     + long data_offset
  #     + UTFString extension
  #   * {File Data} fileCount entries with the raw data of the corresponding file
  #
  # The fileCount integer indicates how many files are contained in this compound
  # file. The {directory} that follows has that many entries. Each directory entry
  # contains an encoding identifier, a long pointer to the start of this file's
  # data section, and a UTF String with that file's extension.
  class CompoundFileWriter

    attr_reader :directory, :file_name

    # Create the compound stream in the specified file. The file name is the
    # entire name (no extensions are added).
    def initialize(dir, name)
      @directory = dir
      @file_name = name
      @ids = Set.new
      @file_entries = []
      @merged = false
    end

    # Add a source stream. _file_name_ is the string by which the 
    # sub-stream will be known in the compound stream.
    # 
    # Throws:: IllegalStateError if this writer is closed
    # Throws:: IllegalArgumentError if a file with the same name
    #          has been added already
    def add_file(file_name)
      if @merged
        raise(IllegalStateError, "Can't add extensions after merge has been called")
      end

      if not @ids.add?(file_name)
        raise(IllegalArgumentError, "File " + file + " already added")
      end

      entry = FileEntry.new(file_name)
      @file_entries << entry
    end

    # Merge files with the extensions added up to now.
    # All files with these extensions are combined sequentially into the
    # compound stream. After successful merge, the source files
    # are deleted.
    #
    # Throws:: IllegalStateException if close() had been called before or
    #          if no file has been added to this object
    def close()

      if @merged
        raise(IllegalStateException, "Merge already performed")
      end

      if @file_entries.empty?
        raise(IllegalStateException, "No entries to merge have been defined")
      end

      @merged = true

      # open the compound stream
      os = nil
      begin
        os = @directory.create_output(@file_name)

        # Write the number of entries
        os.write_vint(@file_entries.size)

        # Write the directory with all offsets at 0.
        # Remember the positions of directory entries so that we can
        # adjust the offsets later
        @file_entries.each do |fe|
          fe.directory_offset = os.pos()
          os.write_long(0)  # for now
          os.write_string(fe.file_name)
        end

        # Open the files and copy their data into the stream.
        # Remember the locations of each file's data section.
        @file_entries.each do |fe|
          fe.data_offset = os.pos()
          copy_file(fe, os)
        end

        # Write the data offsets into the directory of the compound stream
        @file_entries.each do |fe|
          os.seek(fe.directory_offset)
          os.write_long(fe.data_offset)
        end

        # Close the output stream. Set the os to nil before trying to
        # close so that if an exception occurs during the close, the
        # finally clause below will not attempt to close the stream
        # the second time.
        tmp = os
        os = nil
        tmp.close()

      ensure
        if (os != nil)
          begin
            os.close()
          rescue
          end
        end
      end
    end

    private

      # Internal class for holding a file
      class FileEntry

        attr_accessor :file_name, :directory_offset, :data_offset

        def initialize(file_name)
          @file_name = file_name
        end

      end

      # Copy the contents of the file with specified extension into the
      # provided output stream. Use a buffer for moving data
      # to reduce memory allocation.
      def copy_file(source, os)
        is = nil
        begin
          start_ptr = os.pos()

          is = @directory.open_input(source.file_name)
          remainder = length = is.length

          buffer = Ferret::Store::BUFFER.clone
          while (remainder > 0)
            len = [remainder, Ferret::Store::BUFFER_SIZE].min
            is.read_bytes(buffer, 0, len)
            os.write_bytes(buffer, len)
            remainder -= len
          end

          # Verify that remainder is 0
          if (remainder != 0)
            raise(IOError,
              "Non-zero remainder length after copying: " + remainder.to_s +
                " (id: " + source.file_name + ", length: " + length.to_s +
                ", buffer size: " + Ferret::Store::BUFFER_SIZE.to_s + ")")
          end

          # Verify that the output length diff is equal to original file
          end_ptr = os.pos()
          diff = end_ptr - start_ptr
          if (diff != length)
            raise(IOError,
              "Difference in the output file offsets " + diff.to_s +
                " does not match the original file length " + length.to_s)
          end

        ensure
          if (is != nil): is.close() end
        end
      end
  end
end
