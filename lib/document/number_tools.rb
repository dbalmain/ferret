module Ferret
  module Document
    # NOTE:: This helper class should only be used if reading index files
    #        created with Java Lucene
    #
    # Provides support for converting longs to Strings, and back again. The
    # strings are structured so that lexicographic sorting order is preserved.
    # 
    # That is, if l1 is less than l2 for any two longs l1 and l2, then
    # NumberTools.long_to_s(l1) is lexicographically less than
    # NumberTools.long_to_s(l2). (Similarly for "greater than" and "equals".)
    # 
    # This class handles all long values
    class NumberTools
      RADIX = 36
      NEGATIVE_PREFIX = '-'

      # NB: NEGATIVE_PREFIX must be < POSITIVE_PREFIX
      POSITIVE_PREFIX = '0'

      # The following constants are from Java
      LONG_MAX_VALUE =  9223372036854775807
      LONG_MIN_VALUE = -9223372036854775808 

      # NB: This function is used to match the java equivalent. Actually
      # ruby allows much larger numbers than Java so this is just so that we
      # can read the Java Lucene created indexes.
      MIN_STRING_VALUE = NEGATIVE_PREFIX + "0000000000000"
      MAX_STRING_VALUE = POSITIVE_PREFIX + "1y2p0ij32e8e7"

      # The length of the long field
      STR_SIZE = MIN_STRING_VALUE.length()

      # Converts a long to a String suitable for indexing.
      def NumberTools.long_to_s(l)
        if (l == LONG_MIN_VALUE)
            # special case, because long is not symetric around zero
            return MIN_STRING_VALUE;
        end

        s = ""
        if (l < 0)
          s << NEGATIVE_PREFIX
          l = LONG_MAX_VALUE + l + 1
        else
          s << POSITIVE_PREFIX
        end
        num = l.to_s(RADIX)

        pad_len = STR_SIZE - num.length() - s.length()
        while ((pad_len -= 1) >= 0)
            s << '0'
        end
        s << num

        return s
      end

      # Converts a String that was returned by #long_to_s back to a long.
      # 
      # Throws:: ArgumentError if the input is nil
      def NumberTools.s_to_long(s)
        if (s == nil)
          raise ArgumentError, "string cannot be nil"
        end
        if (s.length() != STR_SIZE)
          raise ArgumentError, "string is the wrong size"
        end

        if (s == MIN_STRING_VALUE)
          return LONG_MIN_VALUE
        end

        prefix = s[0,1]
        l = s[1..-1].to_i(36)

        if (prefix == POSITIVE_PREFIX)
            # nop
        elsif (prefix == NEGATIVE_PREFIX)
            l = l - LONG_MAX_VALUE - 1
        else
          raise ArgumentError, "string <" + prefix +
            "> does not begin with the correct prefix"
        end

        return l
      end
    end
  end
end
