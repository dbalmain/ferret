module Ferret
  module Index
    class SegmentMergeInfo 
      attr_reader :term, :term_enum, :reader, :postings, :doc_map, :base

      def initialize(base, term_enum, reader)
        @base = base
        @reader = reader
        @term_enum = term_enum
        @term = term_enum.term()
        @postings = @reader.term_positions()

        # build array which maps document numbers around deletions 
        if (@reader.has_deletions?()) 
          max_doc = @reader.max_doc()
          @doc_map = Array.new(max_doc)
          j = 0
          max_doc.times do |i|
            if (@reader.deleted?(i))
              @doc_map[i] = -1
            else
              @doc_map[i] = j
              j += 1
            end
          end
        end
      end

      def next?
        if @term_enum.next?
          @term = @term_enum.term
          return true
        else 
          @term = nil
          return false
        end
      end

      def close()
        @term_enum.close()
        @postings.close()
        @reader = nil
      end
    end
  end
end

