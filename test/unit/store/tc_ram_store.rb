require File.dirname(__FILE__) + "/../../test_helper"
require File.dirname(__FILE__) + "/tm_store"
require File.dirname(__FILE__) + "/tm_store_lock"

class RAMStoreTest < Test::Unit::TestCase
  include StoreTest
  include StoreLockTest
  def setup
    @dir = Ferret::Store::RAMDirectory.new
  end

  def teardown
    @dir.close()
  end
end
