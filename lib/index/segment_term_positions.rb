module Ferret::Index
  class SegmentTermPositions < SegmentTermDocs
    def initialize(p) 
      super
      @prox_stream = p.prox_stream.clone()
    end

    def seek(ti)
      super
      if (ti != nil)
        @prox_stream.seek(ti.prox_pointer)
      end
      @prox_count = 0
    end

    def close()
      super
      @prox_stream.close()
    end

    def next_position()
      @prox_count -= 1
      return @position += @prox_stream.read_vint()
    end

    def skipping_doc()
      @freq.times { @prox_stream.read_vint() }
    end

    def next?()
      @prox_count.times { @prox_stream.read_vint() }

      if (super)
        @prox_count = @freq    # note frequency
        @position = 0         # reset position
        return true
      end
      return false
    end

    def read(docs, freqs) 
      raise NotImplementedError, "TermPositions does not support processing multiple documents in one call. Use TermDocs instead."
    end

    # Called by super.skipTo(). 
    def skip_prox(prox_pointer)
      @prox_stream.seek(prox_pointer)
      @prox_count = 0
    end
  end
end
