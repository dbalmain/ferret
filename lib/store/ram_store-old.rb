module Ferret
  module Store
    require 'monitor'

    class RAMDirectory < Directory
      include MonitorMixin

      def initialize(dir = nil, close_dir = false)
        super()
        @files = Hash.new
        unless dir.nil?
          dir.each do |file|
            os = create_output(file)    # make a place on ram disk
            is = dir.open_input(file)   # read the current file
            len = is.length             # and copy the file to ram disk
            buf = Array.new(len)
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
          next if file =~ Regexp.new('^rubylock-')
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
        if @files[name]
          @files[name].buffer = nil
          @files.delete(name)
        end
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
        if @files[name].nil?
          each() {|file| puts file.name}
          raise IOError, "No file #{name}"
        end
        RAMIndexInput.new(@files[name])
      end
      
      # Construct a Lock.
      def make_lock(name) 
        RAMLock.new("rubylock-" + name, self)
      end


      # Closes the store.
      def close()
      end

      def to_s
        str = "The files in this directory are: \n"
        @files.each do |path,file|
          str << path + " - " + file.size.to_s + "\n"
        end
        str
      end
   
      class RAMIndexOutput < BufferedIndexOutput
        def initialize(f = nil)
          @file = f || RAMFile.new("")
          @pointer = 0
          super()
        end
        
        def length
          return @file.length
        end

        def reset
          seek(0)
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
          data = @file.buffer
          output.write_bytes(data, data.length)
        end

        private

          def flush_buffer(src, len)
            @file.buffer[@pointer, len] = src[0, len]
            @pointer += len
            @file.mtime = Time.now
          end

      end
      
      class RAMIndexInput < BufferedIndexInput
        attr_reader :file

        def initialize(f)
          @pointer = 0
          @file = f
          super()
        end
        
        def length
          return @file.length
        end

        def close
        end

        def print
          puts @file.buffer.pack("U*")
        end

        private

          def read_internal(b, offset, len)
            b[offset, len] = @file.buffer[@pointer, len]
            @pointer += len
          end

          def seek_internal(pos)
            @pointer = pos
          end
      end
      
      # This class contains an array of byte arrays which act as buffers to
      # store the data in.
      class RAMFile
        attr_accessor :buffer
        attr_accessor :mtime
        attr_accessor :name
        alias :path :name

        def length
          return @buffer.length
        end

        def initialize(name)
          @buffer = Array.new
          @mtime = Time.now
          @name = name
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
            @dir.synchronize do
              # create a file if none exists. If one already exists
              # then someone beat us to the lock so return false
              if (! locked?) then
                @dir.create_output(@lock_file)
                return true
              end
            end
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
end
