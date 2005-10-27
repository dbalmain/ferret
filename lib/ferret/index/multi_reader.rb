module Ferret::Index
  # An IndexReader which reads multiple indexes, appending their content.
  class MultiReader < IndexReader 
    attr_reader :max_doc

    # Construct a MultiReader aggregating the named set of (sub)readers.
    # Directory locking for delete, undeleteAll, and set_norm operations is
    # left to the subreaders. 
    #
    # Note that all subreaders are closed if this Multireader is closed.
    # sub_readers:: set of (sub)readers
    # raises:: IOException
    def initialize(sub_readers, directory = nil, sis = nil, close_dir = false)
      if (directory)
        super(directory, sis, close_dir)
      else
        super(sub_readers.length == 0 ? nil : sub_readers[0].directory())
      end

      @max_doc = 0
      @num_docs = -1
      @has_deletions = false

      @sub_readers = sub_readers
      @starts = Array.new(@sub_readers.length + 1)    # build starts array
      @sub_readers.each_with_index do |sub_reader, i|
        @starts[i] = @max_doc
        @max_doc += sub_reader.max_doc # compute max_docs

        if @sub_readers[i].has_deletions?
          @has_deletions = true
        end
      end
      @starts[@sub_readers.length] = @max_doc
      @norms_cache = {}
    end


    # Return an array of term frequency vectors for the specified document.  The
    # array contains a vector for each vectorized field in the document.  Each
    # vector vector contains term numbers and frequencies for all terms in a
    # given vectorized field.  If no such fields existed, the method returns
    # nil.
    def get_term_vectors(n)
      i = reader_index(n)        # find segment num
      return @sub_readers[i].get_term_vectors(n - @starts[i]); # dispatch to segment
    end

    def get_term_vector(n, field)
       i = reader_index(n)        # find segment num
      return @sub_readers[i].get_term_vector(n - @starts[i], field)
    end

    def num_docs() 
      synchronize do 
        if (@num_docs == -1) # check cache
          n = 0                # cache miss -= 1recompute
          @sub_readers.each {|reader| n += reader.num_docs()}
          @num_docs = n
        end
        return @num_docs
      end
    end

    def get_document(n)
      i = reader_index(n)                                 # find segment num
      return @sub_readers[i].get_document(n - @starts[i]) # dispatch to segment reader
    end

    def deleted?(n) 
      i = reader_index(n)                             # find segment num
      return @sub_readers[i].deleted?(n - @starts[i]) # dispatch to segment reader
    end

    def has_deletions?()
      return @has_deletions
    end

    def do_delete(n)
      @num_docs = -1                         # invalidate cache
      i = reader_index(n)                    # find segment num
      @sub_readers[i].delete(n - @starts[i]) # dispatch to segment reader
      @has_deletions = true
    end

    def do_undelete_all()
      @num_docs = -1                         # invalidate cache
      @sub_readers.each {|reader| reader.undelete_all() }
      @has_deletions = false
    end

    def reader_index(n) # find reader for doc n:
      lo = 0                       # search @starts array
      hi = @sub_readers.length - 1 # for first element less

      while (hi >= lo) 
        mid = (lo + hi) >> 1
        mid_value = @starts[mid]
        if (n < mid_value)
          hi = mid - 1
        elsif (n > mid_value)
          lo = mid + 1
        else # found a match
          while (mid+1 < @sub_readers.length and @starts[mid+1] == mid_value) 
            mid += 1 # scan to last match
          end
          return mid
        end
      end
      return hi
    end

    def get_norms(field)
      synchronize do
        bytes = @norms_cache[field]
        if (bytes != nil)
          return bytes    # cache hit
        end

        bytes = " " * @max_doc
        @sub_readers.length.times do |i|
          @sub_readers[i].get_norms_into(field, bytes, @starts[i])
        end
        @norms_cache[field] = bytes      # update cache
        return bytes
      end
    end

    def get_norms_into(field, buf, offset)
      synchronize do
        bytes = @norms_cache[field]
        if (bytes != nil)                            # cache hit
          buf[offset ,@max_doc] = bytes[0, @max_doc]
          return
        end

        @sub_readers.length.times do |i|
          @sub_readers[i].get_norms_into(field, buf, offset + @starts[i])
        end
      end
    end

    def do_set_norm(n, field, value)
      @norms_cache.delete(field)                   # clear cache
      i = reader_index(n)                          # find segment num
      @sub_readers[i].set_norm(n-@starts[i], field, value); # dispatch
    end

    def terms()
      return MultiTermEnum.new(@sub_readers, @starts, nil)
    end

    def terms_from(term)
      return MultiTermEnum.new(@sub_readers, @starts, term)
    end

    def doc_freq(t)
      total = 0          # sum freqs in segments
      @sub_readers.each {|reader| total += reader.doc_freq(t)}
      return total
    end

    def term_docs()
      return MultiTermDocEnum.new(@sub_readers, @starts)
    end

    def term_positions()
      return MultiTermDocPosEnum.new(@sub_readers, @starts)
    end

    def do_commit()
      @sub_readers.each {|reader| reader.commit() }
    end

    def do_close()
      synchronize do
        @sub_readers.each {|reader| reader.close() }
      end
    end

    # See IndexReader#get_field_names
    def get_field_names(field_option = IndexReader::FieldOption::ALL)
      # maintain a unique set of field names
      field_set = Set.new
      @sub_readers.each do |reader|
        field_set |= reader.get_field_names(field_option)
      end
      return field_set
    end
  end

  class MultiTermEnum < TermEnum 

    attr_reader :doc_freq, :term

    def initialize(readers, starts, t)
      @queue = SegmentMergeQueue.new(readers.length)
      readers.each_index do |i|
        reader = readers[i]
        term_enum = nil
        if (t != nil) 
          term_enum = reader.terms_from(t)
        else
          term_enum = reader.terms()
        end
        smi = SegmentMergeInfo.new(starts[i], term_enum, reader)

        if (t == nil and smi.next?) or term_enum.term
          @queue.push(smi);          # initialize queue
        else
          smi.close()
        end
      end

      if (t != nil and @queue.size() > 0) 
        next?()
      end
    end

    def next?()
      top = @queue.top()
      if (top == nil) 
        @term = nil
        return false
      end

      @term = top.term
      @doc_freq = 0

      while top and @term == top.term
        @queue.pop()
        @doc_freq += top.term_enum.doc_freq() # increment freq
        if (top.next?)
          @queue.push(top) # restore queue
        else
          top.close()     # done with a segment
        end
        top = @queue.top()
      end
      return true
    end

    def close()
      @queue.close()
    end
  end

  class MultiTermDocEnum < TermDocEnum 
    attr_accessor :readers, :starts, :term, :base, :pointer, :current

    def initialize(readers, starts) 
      @readers = readers
      @starts = starts
      @base = 0
      @pointer = 0

      @reader_term_docs = Array.new(readers.length)
    end

    def doc
      return @base + @current.doc()
    end

    def freq
      return @current.freq()
    end

    def seek(term) 
      @term = term
      @base = 0
      @pointer = 0
      @current = nil
    end

    def next?
      if @current and @current.next? 
        return true
      elsif @pointer < @readers.length 
        @base = @starts[@pointer]
        @current = term_docs(@pointer)
        @pointer += 1
        return next?()
      else
        return false
      end
    end

    # Optimized implementation. Unlike the Java version, this method
    # always returns as many results as it can read.
    def read(docs, freqs)
      got = 0
      last_got = 0
      needed = docs.length

      while (true) 
        while @current.nil?
          if @pointer < @readers.length # try next segment
            @base = @starts[@pointer]
            @current = term_docs(@pointer)
            @pointer += 1
          else 
            return got
          end
        end
        got = @current.read(docs, freqs, got)
        if (got == last_got) # none left in segment
          @current = nil
        else # got some
          b = @base        # adjust doc numbers
          (last_got...got).each {|i| docs[i] += b}
          if got == needed
            return got
          else
            last_got = got
          end
        end
      end
    end

    # As yet unoptimized implementation. 
    def skip_to(target)
      begin 
        return false if not next?
      end while target > doc()
      return true
    end

    def term_docs(i)
      return nil if (@term == nil)
      result = @reader_term_docs[i]
      if (result == nil)
        result = @reader_term_docs[i] = term_docs_from_reader(@readers[i])
      end
      result.seek(@term)
      return result
    end

    def term_docs_from_reader(reader)
      return reader.term_docs()
    end

    def close()
      @reader_term_docs.compact.each do |rtd|
        rtd.close()
      end
    end
  end

  class MultiTermDocPosEnum < MultiTermDocEnum
    def initialize(r, s) 
      super(r,s)
    end

    def term_docs_from_reader(reader)
      return reader.term_positions()
    end

    def next_position()
      return @current.next_position()
    end

  end
end
