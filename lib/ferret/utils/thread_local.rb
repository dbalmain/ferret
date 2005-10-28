require 'thread'
class Thread
  def make_deleter
    lambda{|id| @ferret_cache.delete(id)}
  end

  # Set the local value for the thread
  def set_local(key, value)
    @del ||= make_deleter
    @ferret_cache ||= {}
    ObjectSpace.define_finalizer(key, @del)
    @ferret_cache[key.object_id] = value
  end

  # Get the local value for the thread
  def get_local(key)
    return (@ferret_cache ||= {})[key.object_id]
  end

  # Returns the number of local variables stored. Useful for testing.
  def local_size
    return (@ferret_cache ||= {}).size
  end

  def clear_local
    (@ferret_cache ||= {}).clear
  end
end
