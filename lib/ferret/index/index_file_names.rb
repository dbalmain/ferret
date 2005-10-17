module Ferret
  module Index
    # Useful constants representing filenames and extensions used by lucene
    class IndexFileNames 

      # Name of the index segment file 
      SEGMENTS = "segments"
      
      # Name of the index deletable file 
      DELETABLE = "deletable"
      
      # This array contains all filename extensions used by Lucene's index files, with
      # one exception, namely the extension made up from +.f+ + a number.
      # Also note that two of Lucene's files (+deletable+ and
      # +segments+) don't have any filename extension.
      INDEX_EXTENSIONS = [
          "cfs", "fnm", "fdx", "fdt", "tii", "tis", "frq", "prx", "del",
          "tvx", "tvd", "tvf", "tvp"
      ]
      
      # File extensions of old-style index files 
      COMPOUND_EXTENSIONS = [
        "fnm", "frq", "prx", "fdx", "fdt", "tii", "tis"
      ]
      
      # File extensions for term vector support 
      VECTOR_EXTENSIONS = [
        "tvx", "tvd", "tvf"
      ]
      
    end
  end
end
