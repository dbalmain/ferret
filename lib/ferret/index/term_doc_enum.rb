module Ferret::Index
  # TermDocEnum provides an interface for enumerating &lt;document,
  # frequency&gt; pairs for a term. 
  #
  # The document portion names each document containing the term.  Documents
  # are indicated by number.  The frequency portion gives the number of times
  # the term occurred in each document. 
  # 
  # The pairs are ordered by document number.
  #
  # See IndexReader#term_docs
  class TermDocEnum
    # Sets this to the data for a term.
    # The enumeration is reset to the start of the data for this term.
    def seek(term) raise NotImplementedError end

    # Returns the current document number.
    #
    # This is invalid until #next() is called for the first time.
    def doc() raise NotImplementedError end

    # Returns the frequency of the term within the current document. This
    # is invalid until {@link #next()} is called for the first time.
    def freq() raise NotImplementedError end

    # Moves to the next pair in the enumeration.
    # Returns true iff there is such a next pair in the enumeration.
    def next?() raise NotImplementedError end

    # Attempts to read multiple entries from the enumeration, up to length of
    # _docs_.  Document numbers are stored in _docs_, and term
    # frequencies are stored in _freqs_.  The _freqs_ array must be as
    # long as the _docs_ array.
    #
    # Returns the number of entries read.  Zero is only returned when the
    # stream has been exhausted.
    def read(docs, freqs)  raise NotImplementedError end

    # Skips entries to the first beyond the current whose document number is
    # greater than or equal to _target_. 
    # 
    # Returns true iff there is such an entry. 
    # 
    # Some implementations are considerably more efficient than that.
    def skip_to(target)
      while (target > doc())
        return false if not next?()
      end
      return true
    end

    # Frees associated resources.
    def close() raise NotImplementedError end
  end


  class SegmentTermDocEnum < TermDocEnum 
    attr_accessor :parent, :freq_stream, :count, :df, :deleted_docs, :doc, :freq

    def initialize(parent) 
      @parent = parent
      @freq_stream = parent.freq_stream.clone()
      @deleted_docs = parent.deleted_docs
      @skip_interval = parent.term_infos.skip_interval
      @skip_stream = nil
      @doc = 0
    end

    # Find the term, TermEnum or TermInfo in the doc
    #
    # t:: can be a Term, TermEnum of TermInfo object
    def seek(t)
      if t.instance_of?(Term)
        ti = parent.term_infos[t]
      elsif t.is_a?(TermEnum)
        # use comparison of fieldinfos to verify that term enum (t) belongs to the
        # same segment as this SegmentTermDocEnum
        if (t.instance_of?(SegmentTermEnum) and t.field_infos == parent.field_infos)
          ti = t.term_info()
        else                                          # punt case
          ti = parent.term_infos[t.term]
        end
      elsif t.is_a? TermInfo # this one is easy. That's exactly what we're looking for
        ti = t
      else
        raise ArgumentError, "Must pass a Term, TermEnum or TermInfo object, not a " +
          t.class.to_s
      end
      do_seek(ti)
      #puts "pos = #{@freq_stream.pos} ti = #{ti}"
    end
        
    def do_seek(ti)
      @count = 0
      if (ti == nil) 
        @doc_freq = 0
      else 
        @doc_freq = ti.doc_freq
        @doc = 0
        @skip_doc = 0
        @skip_count = 0
        @num_skips = @doc_freq / @skip_interval
        @freq_pointer = ti.freq_pointer
        @prox_pointer = ti.prox_pointer
        @skip_pointer = @freq_pointer + ti.skip_offset
        @freq_stream.seek(@freq_pointer)
        @have_skipped = false
      end
    end

    def close()
      @freq_stream.close()
      if (@skip_stream != nil)
        @skip_stream.close()
      end
      @parent = nil
    end

    def skipping_doc()
    end

    def next?()
      while (true) 
        return false if @count == @doc_freq

        doc_code = @freq_stream.read_vint()
        @doc += doc_code >> 1              # shift off low bit
        if ((doc_code & 1) != 0)           # if low bit is set
          @freq = 1                        # freq is one
        else
          @freq = @freq_stream.read_vint() # else read freq
        end

        @count += 1

        break if (@deleted_docs == nil or not @deleted_docs[@doc])
          
        skipping_doc()
      end
      return true
    end

    # Optimized implementation. 
    def read(docs, freqs, start = 0)
      i = start
      needed=docs.length

      while (i < needed and @count < @doc_freq) 

        # manually inlined call to next() for speed
        doc_code = @freq_stream.read_vint()
        @doc += doc_code >> 1              # shift off low bit
        if ((doc_code & 1) != 0)           # if low bit is set
          @freq = 1                        # freq is one
        else
          @freq = @freq_stream.read_vint() # else read freq
        end
        @count += 1

        if (@deleted_docs == nil or not @deleted_docs[@doc]) 
          docs[i] = @doc
          freqs[i] = @freq
          i += 1
        end
      end
      return i
    end

    # Overridden by SegmentTermDocPosEnum to skip in prox stream. 
    def skip_prox(prox_pointer)
    end

    # Optimized implementation. 
    def skip_to(target)
      if (@doc_freq >= @skip_interval) # optimized case

        if (@skip_stream == nil)
          @skip_stream = @freq_stream.clone() # lazily clone
        end

        if (!@have_skipped) # lazily seek skip stream
          @skip_stream.seek(@skip_pointer)
          @have_skipped = true
        end

        # scan skip data
        last_skip_doc = @skip_doc
        last_freq_pointer = @freq_stream.pos()
        last_prox_pointer = -1
        num_skipped = -1 - (@count % @skip_interval)

        while (target > @skip_doc) 
          last_skip_doc = @skip_doc
          last_freq_pointer = @freq_pointer
          last_prox_pointer = @prox_pointer
          
          if (@skip_doc != 0 and @skip_doc >= @doc)
            num_skipped += @skip_interval
          end
          
          if(@skip_count >= @num_skips)
            break
          end

          @skip_doc += @skip_stream.read_vint()
          @freq_pointer += @skip_stream.read_vint()
          @prox_pointer += @skip_stream.read_vint()

          @skip_count += 1
        end
        
        # if we found something to skip, then skip it
        if (last_freq_pointer > @freq_stream.pos()) 
          @freq_stream.seek(last_freq_pointer)
          skip_prox(last_prox_pointer)

          @doc = last_skip_doc
          @count += num_skipped
        end

      end

      # done skipping, now just scan
      
      begin 
        if not next?
          return false
        end
      end while (target > @doc)
      return true
    end
  end

  class SegmentTermDocPosEnum < SegmentTermDocEnum
    def initialize(p) 
      super
      @prox_stream = p.prox_stream.clone()
    end

    def do_seek(ti)
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
      raise NotImplementedError, "TermDocPosEnum does not support processing multiple documents in one call. Use TermDocEnum instead."
    end

    # Called by super.skipTo(). 
    def skip_prox(prox_pointer)
      @prox_stream.seek(prox_pointer)
      @prox_count = 0
    end
  end
end
