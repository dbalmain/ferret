module Ferret::Utils

  require 'weakref'
  require 'monitor'

  # This class implements a weak key hash. ie all keys that are stored in this
  # hash can still be garbage collected, and if they are garbage collected
  # then the key and it's corresponding value will be deleted from the hash.
  #    eg.
  #    name = "david"
  #    last_names = WeakKeyHash.new()
  #    last_names[name] = "balmain"
  #    puts last_names["david"]  #=>"balmain"
  #    GC.start
  #    puts last_names["david"]  #=>"balmain"
  #    name = nil
  #    GC.start
  #    # the name "david" will now have been garbage collected so it should
  #    # have been removed from the hash
  #    puts last_names["david"]  #=>nil 
  #
  # WeakKeyHash subclasses Monitor so it can be synchronized on.
  #
  # === NOTE
  # Unfortunately the ruby garbage collector is not always predictable so your
  # results may differ but each key should eventually be freed when all other
  # references have been removed and the garbage collector is ready.
  class WeakKeyHash < Monitor
    # Create a new WeakKeyHash.
    def initialize
      super()
      @hash = {}
      @deleter = lambda{|id| @hash.delete(id)}
    end

    # Set the value for the key just like a Hash
    def []=(key, value)
      ObjectSpace.define_finalizer(key, @deleter)
      @hash[key.object_id] = value
    end

    # Get the value for the key
    def [](key)
      return @hash[key.object_id]
    end

    # Return the number of elements in the Hash
    def size
      @hash.size
    end

    # Print a string representation the WeakKeyHash
    def to_s
      buffer = ""
      @hash.each_pair {|key, value| buffer << "<#{ObjectSpace._id2ref(key)}=>#{value}>"}
      return buffer
    end

  end
end
