require File.dirname(__FILE__) + "/../../test_helper"
require File.dirname(__FILE__) + "/tm_store"
require File.dirname(__FILE__) + "/tm_store_lock"

require 'fileutils'

class FSStoreTest < Test::Unit::TestCase
  include Ferret::Store
  include StoreTest
  include StoreLockTest
  def setup
    @dpath = File.expand_path(File.join(File.dirname(__FILE__),
                       '../../temp/fsdir'))
    @dir = FSDirectory.new(@dpath, true)
  end

  def teardown
    @dir.close()
    Dir[File.join(@dpath, "*")].each {|path| begin File.delete(path) rescue nil end}
  end

  def test_fslock
    lock_name = "_file.f1"
    lock_file_path = make_lock_file_path(lock_name)
    assert(! File.exists?(lock_file_path), "There should be no lock file")
    lock = @dir.make_lock(lock_name)
    assert(! File.exists?(lock_file_path), "There should still be no lock file")
    assert(! lock.locked?,                 "lock shouldn't be locked yet")

    lock.obtain

    assert(lock.locked?,                   "lock should now be locked")

    assert(File.exists?(lock_file_path),   "A lock file should have been created")

    assert(@dir.exists?(lfname(lock_name)),"The lock should exist")

    lock.release

    assert(! lock.locked?,                 "lock should be freed again")
    assert(! File.exists?(lock_file_path), "The lock file should have been deleted")
  end

#  def make_and_loose_lock
#    lock = @dir.make_lock("finalizer_lock")
#    lock.obtain
#    lock = nil
#  end
#
#  def test_fslock_finalizer
#    lock_name = "finalizer_lock"
#    lock_file_path = make_lock_file_path(lock_name)
#    assert(! File.exists?(lock_file_path), "There should be no lock file")
#
#    make_and_loose_lock
#
#    #assert(File.exists?(lock_file_path), "There should now be a lock file")
#
#    lock = @dir.make_lock(lock_name)
#    assert(lock.locked?, "lock should now be locked")
#
#    GC.start
#
#    assert(! lock.locked?, "lock should be freed again")
#    assert(! File.exists?(lock_file_path), "The lock file should have been deleted")
#  end
#
  def test_permissions
    _S_IRGRP = 0040
    _S_IWGRP = 0020

    dpath = File.expand_path(File.join(File.dirname(__FILE__),
                       '../../temp/fsdir_permissions'))

    FileUtils.mkdir_p(dpath)
    dstat = File.stat(dpath)

    File.chown(nil, `id -G`.split.last.to_i, dpath)
    File.chmod(dstat.mode | _S_IRGRP | _S_IWGRP, dpath)

    dir = FSDirectory.new(dpath, true)

    file_name = 'test_permissions'
    file_path = File.join(dpath, file_name)

    dir.touch(file_name)

    mode = File.stat(file_path).mode

    assert(mode & _S_IRGRP == _S_IRGRP, "file should be group-readable")
    assert(mode & _S_IWGRP == _S_IWGRP, "file should be group-writable")
  ensure
    if dstat
      File.chown(nil, dstat.gid, dpath)
      File.chmod(dstat.mode, dpath)
    end

    if dir
      dir.refresh()
      dir.close()
    end
  end

  def make_lock_file_path(name)
    lock_file_path = File.join(@dpath, lfname(name))
    if File.exists?(lock_file_path) then
      File.delete(lock_file_path)
    end
    return lock_file_path
  end

  def lfname(name)
    "ferret-#{name}.lck"
  end
end
