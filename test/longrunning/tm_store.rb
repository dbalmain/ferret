module StoreTest
  # declare dir so inheritors can access it.
  def test_modified_full
    # difficult to test this one but as file mtime is only stored to the
    # nearest second. We can assume this test will happen in less than one
    # second. (I hope)
    time = Time.new.to_i
    @dir.touch('mtime_test')
    time_before = @dir.modified('mtime_test').to_i
    assert(time_before - time <= 2, "test that mtime is approximately equal to the system time when the file was touched")
    # wait until the time ticks over one second.
    time = Time.new while (time.to_i == time_before)
    time_before_again = @dir.modified('mtime_test').to_i
    assert_equal(time_before, time_before_again, "the modified time shouldn't change")
    @dir.touch('mtime_test')
    time_after = @dir.modified('mtime_test').to_i
    assert(time_before < time_after, "the modified time should now be greater")
  end
end
