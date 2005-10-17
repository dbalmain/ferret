module Ferret::Search
  require 'monitor'

  # Wraps another filter's result and caches it.  The caching
  # behavior is like QueryFilter.  The purpose is to allow
  # filters to simply filter, and then wrap with this class to add
  # caching, keeping the two concerns decoupled yet composable.
  class CachingWrapperFilter < Filter 
    # filter:: Filter to cache results of
    def initialize(filter) 
      @filter = filter
      @cache = nil
    end

    def bits(reader)
      if (@cache == nil) 
        @cache = WeakKeyHash.new.extend(MonitorMixin)
      end

      @cache.synchronize() do # check cache
        bits = @cache[reader]
        if bits
          return bits
        end
      end

      bits = @filter.bits(reader)

      @cache.synchronize() do # update cache
        @cache[reader] = bits
      end

      return bits
    end

    def to_s() 
      return "CachingWrapperFilter(#{@filter})"
    end
  end
end
