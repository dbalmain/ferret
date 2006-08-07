module Ferret::Store
  require 'monitor'

  class RAMDirectory < Directory
    include MonitorMixin

    def initialize(dir = nil, close_dir = false)
      super()
      @files = Hash.new
      if dir
        buf = BUFFER.clone
        dir.each do |file|
          os = create_output(file)    # make a place on ram disk
          is = dir.open_input(file)   # read the current file
          len = is.length             # and copy the file to ram disk
          if len > buf.size
            buf << " " * (len - buf.size)
          end
          is.read_bytes(buf, 0, len)
          os.write_bytes(buf, len)
          is.close()
          os.close()
        end
        dir.close() if close_dir
      end
    end

    # returns an array of strings, one for each file in the directory
    def each()
      @files.each do |path, file|
        next if file =~ /#{LOCK_PREFIX}/
        yield file
      end
    end

    # Returns true if a file with the given name exists.
    def exists?(name)
      @files.has_key?(name)
    end

    # Returns the time the named file was last modified.
    def modified(name)
      @files[name].mtime
    end

    # Set the modified time of an existing file to now.
    def touch(name)
      if @files[name].nil?
        @files[name] = RAMFile.new(name)
      end
      @files[name].mtime = Time.now
    end

    # Removes an existing file in the directory.
    def delete(name)
      @files.delete(name)
    end

    # Renames an existing file in the directory.
    # If a file already exists with the new name, then it is replaced.
    # This replacement should be atomic.
    def rename(from, to)
      @files[to] = @files[from]
      @files.delete(from)
    end

    # Returns the length of a file in the directory.
    def length(name)
      @files[name].length
    end

    # Creates a new, empty file in the directory with the given name.
    # Returns a stream writing this file.
    def create_output(name)
      file = RAMFile.new(name)
      @files[name] = file
      RAMIndexOutput.new(file)
    end
    
    # Returns a stream reading an existing file.
    def open_input(name)
      raise IOError, "No file #{name}" if @files[name].nil?
      RAMIndexInput.new(@files[name])
    end
    
    def print_file(name)
      input = RAMIndexInput.new(@files[name])
      buf = " " * input.length
      input.read_internal(buf, 0, input.length)
      puts buf
    end

    # Construct a Lock.
    def make_lock(name) 
      RAMLock.new(LOCK_PREFIX + name + ".lck", self)
    end


    # Closes the store.
    def close()
    end

    def to_s
      str = "The files in this directory are: \n"
      @files.each do |path, file|
        str << path + " - " + file.size.to_s + "\n"
      end
      str
    end
 
    class RAMIndexOutput < BufferedIndexOutput
      def initialize(f)
        @file = f
        @pointer = 0
        super()
      end
      
      def length
        return @file.length
      end

      def flush_buffer(src, len)
        buffer_number = (@pointer / BUFFER_SIZE).to_i
        buffer_offset = @pointer % BUFFER_SIZE
        bytes_in_buffer = BUFFER_SIZE - buffer_offset
        bytes_to_copy = [bytes_in_buffer, len].min
        
        extend_buffer_if_necessary(buffer_number)
        
        buffer = @file.buffers[buffer_number]
        buffer[buffer_offset, bytes_to_copy] = src[0, bytes_to_copy]
        
        if bytes_to_copy < len
          src_offset = bytes_to_copy
          bytes_to_copy = len - bytes_to_copy
          buffer_number += 1
          extend_buffer_if_necessary(buffer_number)
          buffer = @file.buffers[buffer_number]
          buffer[0, bytes_to_copy] = src[src_offset, bytes_to_copy]
        end
        @pointer += len
        @file.length = @pointer unless @pointer < @file.length
        @file.mtime = Time.now
      end

      def reset
        seek(0)
        @file.length = 0
      end

      def seek(pos)
        super(pos)
        @pointer = pos
      end

      def close
        super()
        @file.mtime = Time.new
      end

      def write_to(output)
        flush()
        last_buffer_number = (@file.length / BUFFER_SIZE).to_i
        last_buffer_offset = @file.length % BUFFER_SIZE

        (0..last_buffer_number).each do |i|
          len = (i == last_buffer_number ? last_buffer_offset : BUFFER_SIZE)
          output.write_bytes(@file.buffers[i], len)
        end
      end
      
      private

      def extend_buffer_if_necessary(buffer_number)
        if buffer_number == @file.buffers.size
          @file.buffers << RAMFile::BUFFER.clone
        end
      end

    end
    
    class RAMIndexInput < BufferedIndexInput

      def initialize(f)
        @pointer = 0
        @file = f
        super()
      end
      
      def length
        return @file.length
      end

      def read_internal(b, offset, length)
        remainder = length
        start = @pointer
        
        while remainder != 0
          buffer_number = (start / BUFFER_SIZE).to_i
          buffer_offset = start % BUFFER_SIZE
          bytes_in_buffer = BUFFER_SIZE - buffer_offset
          
          if bytes_in_buffer >= remainder
            bytes_to_copy = remainder
          else
            bytes_to_copy = bytes_in_buffer
          end
          buffer = @file.buffers[buffer_number]
          bo2 = buffer_offset
          do2 = offset
          b[do2, bytes_to_copy] = buffer[bo2, bytes_to_copy]
          offset += bytes_to_copy
          start += bytes_to_copy
          remainder -= bytes_to_copy
        end
        
        @pointer += length
      end

      def seek_internal(pos)
        @pointer = pos
      end

      def close
      end
    end
    
    # This class contains an array of byte arrays which act as buffers to
    # store the data in.
    class RAMFile
      BUFFER = " " * BUFFER_SIZE

      attr_reader :buffers
      attr_accessor :mtime
      #attr_accessor :name
      attr_accessor :length
      alias :size :length


      def initialize(name)
        @buffers = Array.new
        @mtime = Time.now
        @length = 0
      end
    end

    # A Lock is used to lock a data source (in this case a file) so that
    # not more than one output stream can access a data source at one time.
    class RAMLock < Lock
      # pass the name of the file that we are going to lock
      def initialize(lock_file, dir)
        @lock_file = lock_file
        @dir = dir
      end

      # obtain the lock on the data source 
      def obtain(lock_timeout = 1) 
        MAX_ATTEMPTS.times do
          #@dir.synchronize do
            # create a file if none exists. If one already exists
            # then someone beat us to the lock so return false
            if (! locked?) then
              @dir.create_output(@lock_file)
              return true
            end
          #end
          # lock was not obtained so sleep for timeout then try again.
          sleep(lock_timeout)
        end
        # lock could not be obtained so raise an exception
        raise "could not obtain lock: " + @lock_file.to_s
      end 

      # Release the lock on the data source. Returns true if successful.
      def release 
        @dir.delete(@lock_file)
        return true
      end

      # returns true if there is a lock on the data source
      def locked?
        @dir.exists?(@lock_file)
      end
    end
  end
end
