require 'monitor'
include Ferret::Utils::StringHelper
module Ferret::Index
# This stores a monotonically increasing set of <Term, TermInfo> pairs in a
# Directory.  A TermInfos can be written once, in order.  

  class TermInfosWriter 
    attr_reader :index_interval, :skip_interval, :out
    attr_writer :other
    # The file format version, a negative number. 
    FORMAT = -2


    # TODO: the default values for these two parameters should be settable
    # from IndexWriter.  However, once that's done, folks will start setting
    # them to ridiculous values and complaining that things don't work well,
    # as with mergeFactor.  So, let's wait until a number of folks find that
    # alternate values work better.  Note that both of these values are
    # stored in the segment, so that it's safe to change these w/o
    # rebuilding all indexes.

    # Expert: The fraction of terms in the "dictionary" which should be
    # stored in RAM.  Smaller values use more memory, but make searching
    # slightly faster, while larger values use less memory and make
    # searching slightly slower.  Searching is typically not dominated by
    # dictionary lookup, so tweaking this is rarely useful.
    #
    # Expert: The fraction of TermDocEnum entries stored in skip
    # tables, used to accellerate TermDocEnum#skipTo(int).  Larger
    # values result in smaller indexes, greater acceleration, but fewer
    # accelerable cases, while smaller values result in bigger indexes, less
    # acceleration and more accelerable cases. More detailed experiments
    # would be useful here. 
    def initialize(dir, segment, fis, interval, is_index = false)
      @index_interval = interval
      @skip_interval = 16
      @last_index_pointer = 0
      @last_term = Term.new("", "")
      @last_term_info = TermInfo.new()
      @size = 0
      @is_index = is_index
      @field_infos = fis
      @out = dir.create_output(segment + (@is_index ? ".tii" : ".tis"))
      @out.write_int(FORMAT)                      # write format
      @out.write_long(0)                          # leave space for size
      @out.write_int(@index_interval)             # write @index_interval
      @out.write_int(@skip_interval)              # write @skip_interval
      unless is_index
        @other = TermInfosWriter.new(dir, segment, fis, interval, true)
        @other.other = self
      end
    end

    # Adds a new <Term, TermInfo> pair to the set.
    # Term must be lexicographically greater than all previous Terms added.
    # TermInfo pointers must be positive and greater than all previous.
    def add(term, term_info)
      if (not @is_index and @last_term > term)
        raise IOError, "term out of order #{term.text} < #{@last_term.text}"
      end
      if (term_info.freq_pointer < @last_term_info.freq_pointer)
        raise IOError, "freq pointer out of order"
      end
      if (term_info.prox_pointer < @last_term_info.prox_pointer)
        raise IOError, "prox pointer out of order"
      end

      if (not @is_index and @size % @index_interval == 0)
        @other.add(@last_term, @last_term_info) # add an index term
      end

      write_term(term)                                 # write term
      @out.write_vint(term_info.doc_freq)              # write doc freq
      @out.write_vlong(term_info.freq_pointer - @last_term_info.freq_pointer)
      @out.write_vlong(term_info.prox_pointer - @last_term_info.prox_pointer)
      @out.write_vint(term_info.skip_offset) if (term_info.doc_freq >= @skip_interval) 

      if (@is_index) 
        @out.write_vlong(@other.out.pos() - @last_index_pointer)
        @last_index_pointer = @other.out.pos() # write pointer
      end

      @last_term_info.set!(term_info)
      @size += 1
    end

    # Called to complete TermInfos creation. 
    def close()
      # clear this threads cache 
      Thread.current["#{self.object_id}-term_enum"] = nil

      @out.seek(4)          # write @size after format
      @out.write_long(@size)
      @out.close()

      @other.close() unless @is_index
    end

    private
      def write_term(term)
        start = StringHelper.string_difference(@last_term.text, term.text)
        length = term.text.length() - start

        @out.write_vint(start)                   # write shared prefix length
        @out.write_vint(length)                  # write delta length
        @out.write_chars(term.text, start, length)  # write delta chars
        @out.write_vint(@field_infos.field_number(term.field_name)) # write field num
        @last_term = term
      end
  end

  
  # This stores a monotonically increasing set of <Term, TermInfo> pairs in a
  # Directory.  Pairs are accessed either by Term or by ordinal position the
  # set.  
  class TermInfosReader 
    include MonitorMixin

    def initialize(dir, seg, fis)
      super()
      @directory = dir
      @segment = seg
      @field_infos = fis

      @orig_enum = SegmentTermEnum.new(@directory.open_input(@segment + ".tis"),
                                       @field_infos, false)
      @size = @orig_enum.size
      @skip_interval = @orig_enum.skip_interval
      @index_enum = SegmentTermEnum.new(@directory.open_input(@segment + ".tii"),
                                       @field_infos, true)
      @index_terms = nil
      @index_infos = nil
      @index_pointers = nil
    end

    def close()
      @orig_enum.close() if (@orig_enum != nil)
      @index_enum.close() if (@index_enum != nil)
    end

    # Returns the number of term/value pairs in the set. 
    attr_reader :size
    # The skip interval for the original enumerator
    attr_reader :skip_interval


    # Returns the TermInfo for a Term in the set, or nil. 
    def get_term_info(term)
      return nil if (@size == 0)

      ensure_index_is_read()

      # optimize sequential access: first try scanning cached enum w/o seeking
      e = enum()
      if e.term and term >= e.term
        enum_offset = (e.position / e.index_interval).to_i + 1
        if (@index_terms.length == enum_offset or
            term < @index_terms[enum_offset]) # but before end of block
          return scan_for_term_info(term)        # no need to seek
        end
      end

      # random-access: must seek
      seek_enum(get_index_offset(term))
      return scan_for_term_info(term)
    end
    alias :[] :get_term_info

    # Returns the nth term in the set. 
    def get_term(position)
      return nil if (@size == 0)

      e = enum()
      if (e != nil and
          e.term != nil and
          position >= e.position and
          position < (e.position + e.index_interval))
        return scan_for_term(position)      # can avoid seek
      end

      seek_enum((position / e.index_interval).to_i) # must seek
      return scan_for_term(position)
    end

    def get_terms_position(term)
      return nil if (@size == 0)
      ensure_index_is_read
      seek_enum(get_index_offset(term))

      e = enum()

      while term > e.term and e.next?
      end

      return term == e.term ? e.position : -1
    end

    # Returns an enumeration of all the Terms and TermInfos in the set. 
    def terms() 
      return @orig_enum.clone()
    end

    # Returns an enumeration of terms starting at or after the named term. 
    def terms_starting_at(term)
      get_position(term)
      return enum().clone()
    end

    private

      def enum() 
        term_enum = Thread.current["#{self.object_id}-term_enum"]
        if (term_enum == nil) 
          term_enum = terms()
          Thread.current["#{self.object_id}-term_enum"] = term_enum 
        end
        return term_enum
      end

      def ensure_index_is_read()
        synchronize() do
          return if @index_terms
          begin
            index_size = @index_enum.size

            @index_terms = Array.new(index_size)
            @index_infos = Array.new(index_size)
            @index_pointers = Array.new(index_size)

            i = 0
            while @index_enum.next?
              @index_terms[i] = @index_enum.term
              @index_infos[i] = @index_enum.term_info
              @index_pointers[i] = @index_enum.index_pointer
              i += 1
            end
          ensure 
            @index_enum.close()
            @index_enum = nil
          end
        end
      end

      # Returns the offset of the greatest index entry which is less than or
      # equal to term.
      def get_index_offset(term) 
        lo = 0            # binary search @index_terms[]
        hi = @index_terms.length - 1

        while (hi >= lo) 
          mid = (lo + hi) >> 1
          delta = term <=> @index_terms[mid]
          if (delta < 0)
            hi = mid - 1
          elsif (delta > 0)
            lo = mid + 1
          else
            return mid
          end
        end
        return hi
      end

      def seek_enum(ind_offset)
        enum().seek(@index_pointers[ind_offset],
            (ind_offset * enum().index_interval) - 1,
            @index_terms[ind_offset],
            @index_infos[ind_offset])
      end

      # Scans within block for matching term. 
      def scan_for_term_info(term)
        e = enum()
        e.scan_to(term)
        if e.term != nil and term == e.term
          return e.term_info()
        else
          return nil
        end
      end

      def scan_for_term(position)
        e = enum()
        while (e.position < position)
          return nil if not e.next?
        end

        return e.term
      end

      # Returns the position of a Term in the set or -1. 
      def get_position(term)
        return -1 if (@size == 0)

        ind_offset = get_index_offset(term)
        seek_enum(ind_offset)

        e = enum()
        while (term > e.term and e.next?)
        end

        if (term == e.term())
          return e.position
        else
          return -1
        end
      end

  end
end
