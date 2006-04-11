require File.dirname(__FILE__) + "/../../test_helper"
require File.dirname(__FILE__) + "/rtm_store"
require File.dirname(__FILE__) + "/rtm_store_lock"

module Ferret::Store

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
end

class FSStoreTest < Test::Unit::TestCase
  include Ferret::Store
  include StoreTest
  include StoreLockTest
  def setup
    @dpath = File.join(File.dirname(__FILE__),
                       '../../temp/fsdir')
    @dir = FSDirectory.new(@dpath, true)
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
    @dir1 = FSDirectory.new(dir_path, true)
    assert(FSDirectory.directory_cache[dir_path],
           "this directory should now be cached") 
    assert_equal(@dir1.ref_count, 1,
                 "There is one reference so the refcount should now be 1")
    @dir2 = FSDirectory.new(dir_path, true)
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

end