module Ferret::Utils

  require 'weakref'

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
  # ===NOTE
  # Unfortunately the ruby garbage collector is not always predictable so your
  # results may differ but each key should eventually be freed when all other
  # references have been removed and the garbage collector is ready.
  class WeakKeyHash
    def initialize
      @hash = {}
      @deleter = lambda{|id| @hash.delete(id)}
    end

    def []=(key, value)
      ObjectSpace.define_finalizer(key, @deleter)
      @hash[key.object_id] = value
    end

    def [](key)
      return @hash[key.object_id]
    end

    def size
      @hash.size
    end

    def to_s
      buffer = ""
      @hash.each_pair {|key, value| buffer << "<#{ObjectSpace._id2ref(key)}=>#{value}>"}
      return buffer
    end

  end
end
