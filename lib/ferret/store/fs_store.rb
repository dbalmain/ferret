module Ferret::Store

  require 'monitor'
  require 'fileutils'
  require 'digest/md5'

  # This is a filesystem implementation of Directory and will be the one
  # usually used for storing the index. This implementation stores each
  # separate file as a separate file on the operating system. This works fine
  # and is the most efficient solution for small to medium size indexes. For
  # very large indexes, there may be a problem with the operating system not
  # wanting to open to many files. One fix for this is to change the maximum
  # open files setting in your operating system.  Alternatively you could use
  # a compound file instead.
  # 
  # TODO:
  # * need a better way of setting properties. Currently you have to
  #   change the constants to disable locking.
  class FSDirectory < Directory
    include MonitorMixin

    # This cache of directories ensures that there is a unique Directory
    # instance per path, so that synchronization on the Directory can be used to
    # synchronize access between readers and writers.
    @@Directories = Hash.new.extend(MonitorMixin)


    # Locks should be disabled it there is no need for them
    LOCKS_DISABLED = false

    # The lock dir is the directory where the file locks will be stored
    LOCK_DIR = nil

    # Create a new directory from the path.
    # path:: the path to the directory.
    # create:: if true, create, or erase any existing contents.
    def initialize(path, create)
      super()
      if create then FileUtils.mkdir_p(path) end
      if not File.directory?(path) then
        raise IOError, "There is no directory: #{path}. Use create = true to create one"
      end
      @dir = Dir.new(path)
      # put the lock_dir here as well if no default exists.
      if LOCK_DIR then
        @lock_dir = Dir.new(LOCK_DIR) 
      else
        @lock_dir = Dir.new(path) 
      end
      @ref_count = 0
    end

    class <<FSDirectory
      alias :allocate :new
      protected :allocate
    end

    # Returns the directory instance for the named location.
    #
    # Directories are cached, so that, for a given canonical path, the same
    # FSDirectory instance will always be returned.  This permits
    # synchronization on directories.
    #
    # path:: the path to the directory.
    # create:: if true, create, or erase any existing contents.
    def FSDirectory.new(path, create = false)
      dir = nil
      @@Directories.synchronize do
        dir = @@Directories[path]
        if not dir then
          dir = FSDirectory.allocate(path, create)
          @@Directories[path] = dir
        end
        dir.refresh if create
      end
      dir.synchronize do
        dir.reference()
      end
      return dir
    end

    # Returns true if locks have been disabled
    def FSDirectory.locks_disabled?
      LOCKS_DISABLED
    end

    # Set the directory where all of the locks will be stored.
    # path:: the path to the directory where the locks will be stored.
    #        An exception will be raised if the directory does not exist
    def lock_dir=(path)
      # close the old lock dir if it exists
      @lock_dir.close() if @lock_dir
      @lock_dir = Dir.new(path)
    end

    # Returns a Dir object of the directory where the lock is stored
    attr_reader :lock_dir

    # Remove all files and locks from this directory so we have a clean instance
    def refresh
      synchronize do
        # delete all the files
        refresh_dir
        each do |fname|
          File.delete(dir_path(fname))
        end
        # clear all the locks
        refresh_lock_dir
        @lock_dir.each do |lock_fname|
          next if lock_fname == '.' or lock_fname == '..'
          File.delete(@lock_dir.path + '/' + lock_fname)
        end
      end
    end

    #--
    # Directory implementation
    #++

    # Iterates through the file listing, skipping lock files if they exist
    def each()
      refresh_dir
      @dir.each do |file_name|
        # return all files except for the current and parent directories
        # and any lock files that exist in this directory
        next if ['.', '..'].include?(file_name)
        next if file_name =~ Regexp.new('^' + lock_prefix)
        yield file_name
      end
    end

    # Returns true if a file with the given name exists.
    def exists?(name)
      File.exists?(dir_path(name))
    end

    # Returns the time the named file was last modified.
    def modified(name) 
      File.mtime(dir_path(name))
    end

    # Set the modified time of an existing file to now.
    def touch(name) 
      # just open the file and close it. No need to do anything with it.
      FileUtils.touch(dir_path(name))
    end

    # Removes an existing file in the directory.
    def delete(name)
      begin
        File.delete(dir_path(name))
      rescue SystemCallError => e
        raise IOError, e.to_s
      end
    end

    # Renames an existing file in the directory.
    # If a file already exists with the new name, then it is replaced.
    # This replacement should be atomic.
    def rename(from, to)
      synchronize do
        File.rename(dir_path(from), dir_path(to))
      end
    end


    # Returns the length of a file in the directory.
    def length(name)
      File.size(dir_path(name))
    end

    # Creates a new, empty file in the directory with the given name.
    # Returns a stream writing this file.
    def create_output(name) 
      FSIndexOutput.new(dir_path(name))
    end

    # Returns a stream reading an existing file.
    def open_input(name)
      FSIndexInput.new(dir_path(name))
    end

    # Construct a Lock.
    def make_lock(name) 
      FSLock.new(@lock_dir.path + "/" + lock_prefix() + name)
    end

    # Closes the store.
    def close()
      synchronize do
        @ref_count -= 1
        if (@ref_count <= 0) then
          @@Directories.synchronize do
            @@Directories.delete(@dir.path)
            close_internal
          end
        end
      end
    end

    def reference()
      @ref_count += 1
    end

    # See Lock for hints as to how to use locks.
    class FSLock < Lock
      # pass the name of the file that we are going to lock
      def initialize(lock_file)
        @lock_file = lock_file
        #@clean = FSLock.make_finalizer(lock_file)
        @clean = lambda { File.delete(lock_file) rescue nil}
      end

      def FSLock.make_finalizer(lock_file)
        lambda { File.delete(lock_file) rescue nil}
      end

      # obtain the lock on the data source 
      def obtain(lock_timeout = 1) 
        return true if FSDirectory.locks_disabled?
        MAX_ATTEMPTS.times do
          begin
            # create a file if none exists. If one already exists
            # then someone beat us to the lock so return false
            File.open(@lock_file, File::WRONLY|File::EXCL|File::CREAT) {|f|}
            ObjectSpace.define_finalizer(self, @clean)
            return true
          rescue SystemCallError
            # lock was not obtained so sleep for timeout then try again.
            sleep(lock_timeout)
          end
        end
        # lock could not be obtained so raise an exception
        raise "could not obtain lock: #{@lock_file}"
      end 

      # Release the lock on the data source. Returns true if successful.
      def release 
        return if FSDirectory.locks_disabled?
        begin
          File.delete(@lock_file)
          ObjectSpace.undefine_finalizer(self)
        rescue SystemCallError
          # maybe we tried to release a lock that wasn't locked. This
          # isn't critical so just return false
          return false
        end
        return true
      end

      # returns true if there is a lock on the data source
      def locked?
        return false if FSDirectory.locks_disabled?
        File.exists?(@lock_file)
      end
    end

    # A file system output stream extending OutputStream to read from the file
    # system
    class FSIndexOutput < BufferedIndexOutput
      def initialize(path)
        super()
        @file = File.open(path, "wb")
      end

      def close
        super()
        @file.close
      end

      def seek(pos)
        super(pos)
        @file.seek(pos)
      end

      private
        def flush_buffer(b, size)
          @file.syswrite(b[0...size])
        end
    end

    # A file system input stream extending InputStream to read from the file system
    class FSIndexInput < BufferedIndexInput
      attr_accessor :is_clone
      attr_reader   :length, :file

      def initialize(path)
        @file = File.open(path, "rb")
        @file.extend(MonitorMixin)
        #class <<@file
        #  attr_accessor :ref_count
        #end
        #@file.ref_count = 1
        @length = File.size(path)
        @is_clone = false
        super()
      end

      def close
        #@file.ref_count -= 1
        #@file.close if @file.ref_count == 0
        @file.close if not @is_clone
      end

      # We need to record if this is a clone so we know when to close the file.
      # The file should only be closed when the original FSIndexInput is closed.
      def initialize_copy(o)
        super
        @is_clone = true
      end

      private

        def read_internal(b, offset, length)
          @file.synchronize do
            position = pos()
            if position != @file.pos
              @file.seek(position)
            end
            bytes = @file.read(length)
            if bytes.nil?
              raise EOFError, "Read past EOF in #{@file.path}"
            end
            b[offset, bytes.length] = bytes
          end
        end

        def seek_internal(pos)
          @file.seek(pos)
        end

    end

    private

      # Add the directory path to the file name for opening
      def dir_path(name)
        File.join(@dir.path, name)
      end

      # returns the lock prefix for this directory
      def lock_prefix
        LOCK_PREFIX + Digest::MD5.hexdigest(@dir.path)
      end

      # Unfortunately, on Windows, Dir does not refresh when rewind is called
      # so any new files will be hidden. So we open the directory again.
      def refresh_dir()
        tmp = Dir.new(@dir.path)
        @dir.close
        @dir = tmp
      end

      def refresh_lock_dir()
        tmp = Dir.new(@lock_dir.path)
        @lock_dir.close
        @lock_dir = tmp
      end

      # This method is only used by the c extension to free the directory
      def close_internal
      end
    #end private
  end
end
