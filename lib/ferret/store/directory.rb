module Ferret::Store
  # A Directory is an object which is used to access the index storage.
  # Ruby's IO API is not used so that we can use different storage
  # mechanisms to store the index. Some examples are;
  # 
  # * File system based storage
  # * RAM based storage
  # * Database based storage
  #
  # NOTE: Once a file has been written and closed, it can no longer be
  # modified. To make any changes to the file it must be deleted and
  # rewritten. For this reason, the method to open a file for writing is
  # called _create_output_, while the method to open a file for reading is
  # called _open_input_ If there is a risk of simultaneous modifications of
  # the files then locks should be used. See Lock to find out how.
  class Directory
    LOCK_PREFIX = "ferret-"

    # returns an array of strings, one for each file in the directory
    def each # :yeilds: file_name
      raise NotImplementedError
    end

    # returns the number of files in the directory
    def file_count() 
      i = 0
      each {|f| i += 1}
      return i
    end

    # Returns true if a file with the given name exists.
    def exists?(file)
      raise NotImplementedError
    end

    # Returns the time the named file was last modified.
    def modified(file)
      raise NotImplementedError
    end

    # Set the modified time of an existing file to now.
    def touch(file)
      raise NotImplementedError
    end

    # Removes an existing file in the directory.
    def delete(file)
      raise NotImplementedError
    end

    # Renames an existing file in the directory.
    # If a file already exists with the new name, then it is replaced.
    # This replacement should be atomic.
    def rename(from, to)
      raise NotImplementedError
    end

    # Returns the length of a file in the directory.
    def length(file)
      raise NotImplementedError
    end

    # Creates a new, empty file in the directory with the given name.
    # Returns a stream writing this file.
    def create_output(file_name)
      raise NotImplementedError
    end

    # Returns a stream reading an existing file.
    def open_input(file_name)
      raise NotImplementedError
    end

    # Construct a Lock.
    def make_lock(lock_name)
      raise NotImplementedError
    end

    # Closes the store.
    def close
      raise NotImplementedError
    end

  end

  # A Lock is used to lock a data source so that not more than one 
  # output stream can access a data source at one time. It is possible
  # that locks could be disabled. For example a read only index stored
  # on a CDROM would have no need for a lock.
  #
  # You can use a lock in two ways. Firstly:
  #
  #   write_lock = @directory.make_lock(LOCK_NAME)
  #   write_lock.obtain(WRITE_LOCK_TIME_OUT)
  #     ... # Do your file modifications # ...
  #   write_lock.release()
  #
  # Alternatively you could use the while locked method. This ensures that
  # the lock will be released once processing has finished.
  #
  #   write_lock = @directory.make_lock(LOCK_NAME)
  #   write_lock.while_locked(WRITE_LOCK_TIME_OUT) do
  #     ... # Do your file modifications # ...
  #   end
  class Lock
    # Attempts made to obtain the lock before the application gives up. If
    # you want the process to wait longer to get the lock then just increase
    # the lock timeout
    MAX_ATTEMPTS = 5

    # Obtain the lock on the data source. If you expect to have to wait for
    # a while on a lock then you should set the lock_timeout to a large
    # number. This may be necessary if you are doing multiple large batch
    # updates on an index but the default 1 second should be fine in most
    # cases.
    def obtain(lock_timeout = 1)
      raise NotImplementedError
    end

    # Release the lock on the data source
    def release
      raise NotImplementedError
    end

    # Returns true if there is a lock on the data source
    def locked?
      raise NotImplementedError
    end

    # Obtains the lock, processes the block and ensures that the lock is
    # released when the block terminates. The lock timeout is in seconds.
    def while_locked(lock_timeout=1)
      obtain(lock_timeout)
      begin
        yield
      ensure
        release()
      end
    end
  end
end
