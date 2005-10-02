module Ferret::Index
  # A TermInfo is the record of information stored for a term.
  class TermInfo
    attr_accessor :doc_freq, :freq_pointer, :prox_pointer, :skip_offset

    def initialize(df=0, fp=0, pp=0, so=0)
      set_values!(df, fp, pp, so)
    end

    def set!(ti)
      @doc_freq = ti.doc_freq
      @freq_pointer = ti.freq_pointer
      @prox_pointer = ti.prox_pointer
      @skip_offset = ti.skip_offset
    end
    
    def set_values!(df=0, fp=0, pp=0, so=0)
      @doc_freq = df
      @freq_pointer = fp
      @prox_pointer = pp
      @skip_offset = so
    end

    def copy_of()
      TermInfo.new(doc_freq, freq_pointer, prox_pointer, skip_offset)
    end

    def ==(o)
      return false if !o.instance_of?(TermInfo)
      @doc_freq == o.doc_freq &&
        @freq_pointer == o.freq_pointer &&
        @prox_pointer == o.prox_pointer &&
        @skip_offset == o.skip_offset
    end
    alias eql? ==
  end

  def to_s()
    "TermInfo:df=#{@doc_freq}:fp=#{@freq_pointer}:pp=#{@prox_pointer}:so=#{@skip_offset}"
  end
end
