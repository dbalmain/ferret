module Ferret::Index
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
    def read(docs, freqs)
      length = docs.length
      i = 0
      while (i < length and @count < @doc_freq) 

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
end
