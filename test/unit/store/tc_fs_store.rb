require File.dirname(__FILE__) + "/../../test_helper"
require File.dirname(__FILE__) + "/tm_store"
require File.dirname(__FILE__) + "/tm_store_lock"
include Ferret::Store

class FSDirectory
  def FSDirectory.directory_cache
    @@Directories
  end

  def ref_count
    @ref_count
  end

  def get_lock_prefix
    lock_prefix
  end
end

class FSStoreTest < Test::Unit::TestCase
  include StoreTest
  include StoreLockTest
  def setup
    @dpath = File.join(File.dirname(__FILE__),
                       '../../temp/fsdir')
    @dir = FSDirectory.get_directory(@dpath, true)
  end

  def teardown
    @dir.refresh()
    @dir.close()
  end

  def test_cache
    dir_path = File.join(File.dirname(__FILE__),
                         '/../../temp/cachetest')
    assert(! FSDirectory.directory_cache[dir_path],
           "this directory should not be cached yet") 
    @dir1 = FSDirectory.get_directory(dir_path, true)
    assert(FSDirectory.directory_cache[dir_path],
           "this directory should now be cached") 
    assert_equal(@dir1.ref_count, 1,
                 "There is one reference so the refcount should now be 1")
    @dir2 = FSDirectory.get_directory(dir_path, true)
    assert(@dir1 === @dir2,
           "The directory should be cached so the same directory object should have been returned")
    assert_equal(@dir1.ref_count, 2,
                 "There are two references so the refcount should now be 2")
    @dir1.close
    assert(FSDirectory.directory_cache[dir_path],
           "this directory shouldn't have been removed yet") 
    assert_equal(@dir2.ref_count, 1,
                 "There is one reference so the refcount should now be 1")
    @dir2.close
    assert(! FSDirectory.directory_cache[dir_path],
           "this directory should have been removed from the cache") 
  end

  def test_fslock
    name = "lfile"
    lock_file_path = @dpath + "/" + @dir.get_lock_prefix() + name
    if File.exists?(lock_file_path) then
      File.delete(lock_file_path)
    end
    assert(! File.exists?(lock_file_path),
           "There should be no lock file")
    lock = @dir.make_lock(name)
    assert(! File.exists?(lock_file_path),
           "There should still be no lock file")
    assert(! @dir.exists?(lock_file_path),
           "The lock should be hidden by the FSDirectories directory scan")
    assert(! lock.locked?, "lock shouldn't be locked yet")
    lock.obtain
    assert(lock.locked?, "lock should now be locked")
    assert(File.exists?(lock_file_path),
           "A lock file should have been created")
    lock.release
    assert(! lock.locked?, "lock should be freed again")
    assert(! File.exists?(lock_file_path), "The lock file should have been deleted")
  end
end
