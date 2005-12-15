require 'monitor'

module Ferret::Search
  # Subclass of FilteredTermEnum for enumerating all terms that are similiar
  # to the specified filter term.
  # 
  # Term enumerations are always ordered by Term.compareTo().  Each term in
  # the enumeration is greater than all that precede it.
  class FuzzyTermEnum < FilteredTermEnum 
    include MonitorMixin

    include Ferret::Index
    attr_reader :end_enum

    # This should be somewhere around the average long word.
    # If it is longer, we waste time and space. If it is shorter, we waste a
    # little bit of time growing the array as we encounter longer words.
    TYPICAL_LONGEST_WORD_IN_INDEX = 19

    # Constructor for enumeration of all terms from specified +reader+ which
    # share a prefix of length +prefix_length+ with +term+ and which have a
    # fuzzy similarity > +min_similarity+.
    # 
    # After calling the constructor the enumeration is already pointing to the
    # first valid term if such a term exists. 
    # 
    # reader:: Delivers terms.
    # term:: Pattern term.
    # min_similarity:: Minimum required similarity for terms from the reader.
    # Default value is 0.5.
    # prefix_length:: Length of required common prefix. Default value is 0.
    def initialize(reader, term,
                   minimum_similarity = FuzzyQuery.default_min_similarity,
                   prefix_length = FuzzyQuery.default_prefix_length)
      super()
      
      @reader = reader
      @end_enum = false
      @max_distances = Array.new(TYPICAL_LONGEST_WORD_IN_INDEX)

      
      if (minimum_similarity >= 1.0)
        raise ArgumentError, "minimum_similarity cannot be greater than or equal to 1"
      elsif (minimum_similarity < 0.0)
        raise ArgumentError, "minimum_similarity cannot be less than 0"
      end
      if(prefix_length < 0)
        raise ArgumentError, "prefix_length cannot be less than 0"
      end

      @minimum_similarity = minimum_similarity
      @scale_factor = 1.0 / (1.0 - @minimum_similarity)
      @search_term = term
      @field = @search_term.field

      # The prefix could be longer than the word.
      # It's kind of silly though.  It means we must match the entire word.
      term_length = @search_term.text.length
      if prefix_length > term_length 
        @prefix_length = term_length 
      else
        @prefix_length = prefix_length
      end

      @text = @search_term.text[@prefix_length..-1]
      @prefix = @search_term.text[0, @prefix_length]

      initialize_max_distances()

      # Allows us save time required to create a new array
      # everytime similarity is called.
      @d = init_distance_array()

      self.enum = reader.terms_from(Term.new(@search_term.field, @prefix))
    end

    # The term_compare method in FuzzyTermEnum uses Levenshtein distance to 
    # calculate the distance between the given term and the comparing term. 
    def term_compare(term) 
      if (@field == term.field and term.text[0, @prefix_length] == @prefix) 
        target = term.text[@prefix_length..-1]
        @similarity = similarity(target)
        return (@similarity > @minimum_similarity)
      end
      @end_enum = true
      return false
    end
    
    def difference() 
      return  (@scale_factor * (@similarity - @minimum_similarity))
    end
    
    # ****************************
    # Compute Levenshtein distance
    # ****************************
    
    # Finds and returns the smallest of three integers 
    def min(a, b, c) 
      t = (a < b) ? a : b
      return (t < c) ? t : c
    end

    def init_distance_array()
      return Array.new(@text.length() + 1) {Array.new(TYPICAL_LONGEST_WORD_IN_INDEX)}
    end

    # Similarity returns a number that is 1.0 or less (including negative
    # numbers) based on how similar the Term is compared to a target term.  It
    # returns exactly 0.0 when
    # 
    #    edit_distance < maximum_edit_distance
    #
    # Otherwise it returns:
    #
    #    1 - (edit_distance / length)
    #
    # where length is the length of the shortest term (text or target)
    # including a prefix that are identical and edit_distance is the
    # Levenshtein distance for the two words.
    # 
    # Embedded within this algorithm is a fail-fast Levenshtein distance
    # algorithm.  The fail-fast algorithm differs from the standard
    # Levenshtein distance algorithm in that it is aborted if it is discovered
    # that the mimimum distance between the words is greater than some
    # threshold.
    # 
    # To calculate the maximum distance threshold we use the following formula:
    #
    #    (1 - minimum_similarity) * length
    #
    # where length is the shortest term including any prefix that is not part
    # of the similarity comparision.  This formula was derived by solving for
    # what maximum value of distance returns false for the following
    # statements:
    # 
    #    similarity = 1 - (distance / (prefix_length + [textlen, targetlen].min))
    #    return (similarity > minimum_similarity)
    #
    # where distance is the Levenshtein distance for the two words.
    # 
    # Levenshtein distance (also known as edit distance) is a measure of
    # similiarity between two strings where the distance is measured as the
    # number of character deletions, insertions or substitutions required to
    # transform one string to the other string.
    #
    # target:: the target word or phrase
    # returns:: the similarity,  0.0 or less indicates that it matches less
    #    than the required threshold and 1.0 indicates that the text and
    #    target are identical
    def similarity(target) 
      synchronize do
        m = target.length
        n = @text.length

        if (n == 0)  
          # we don't have anything to compare.  That means if we just add the
          # letters for m we get the new word
          return (@prefix_length == 0) ? 0.0 : 1.0 - (m.to_f / @prefix_length)
        end
        if (m == 0) 
          return (@prefix_length == 0) ? 0.0 : 1.0 - (n.to_f / @prefix_length)
        end

        max_distance = max_distance(m)

        if (max_distance < (m-n).abs) 
          #just adding the characters of m to n or vice-versa results in
          #too many edits
          #for example "pre" length is 3 and "prefixes" length is 8.  We can see that
          #given this optimal circumstance, the edit distance cannot be less than 5.
          #which is 8-3 or more precisesly Math.abs(3-8).
          #if our maximum edit distance is 4, then we can discard this word
          #without looking at it.
          return 0.0
        end

        #let's make sure we have enough room in our array to do the distance calculations.
        if (@d[0].length <= m) 
          grow_distance_array(m)
        end

        # init matrix d
        (n+1).times {|i| @d[i][0] = i}
        (m+1).times {|j| @d[0][j] = j}
        
        # start computing edit distance
        1.upto(n) do |i|
          best_possible_edit_distance = m
          s_i = @text[i-1]
          1.upto(m) do |j|
            if (s_i != target[j-1]) 
              @d[i][j] = min(@d[i-1][j], @d[i][j-1], @d[i-1][j-1])+1
            else 
              @d[i][j] = min(@d[i-1][j]+1, @d[i][j-1]+1, @d[i-1][j-1])
            end
            if @d[i][j] < best_possible_edit_distance
              best_possible_edit_distance = @d[i][j]
            end
          end

          # After calculating row i, the best possible edit distance can be
          # found by found by finding the smallest value in a given column.
          # If the best_possible_edit_distance is greater than the max distance,
          # abort.
          if (i > max_distance and best_possible_edit_distance > max_distance)
            # equal is okay, but not greater
            # the closest the target can be to the text is just too far away.
            # this target is leaving the party early.
            return 0.0
          end
        end

        # this will return less than 0.0 when the edit distance is
        # greater than the number of characters in the shorter word.
        # but this was the formula that was previously used in FuzzyTermEnum,
        # so it has not been changed (even though minimum_similarity must be
        # greater than 0.0)
        return 1.0 - (@d[n][m].to_f / (@prefix_length + (n < m ? n : m)))
      end
    end

    # Grow the second dimension of the array, so that we can calculate the
    # Levenshtein difference.
    def grow_distance_array(m) 
      @d = @d.map {Array.new(m+1)}
    end

    # The max Distance is the maximum Levenshtein distance for the text
    # compared to some other value that results in score that is
    # better than the minimum similarity.
    # m:: the length of the "other value"
    # returns:: the maximum levenshtein distance that we care about
    def max_distance(m) 
      return @max_distances[m] ||= calculate_max_distance(m)
    end

    def initialize_max_distances() 
      @max_distances.length.times do |i|
        @max_distances[i] = calculate_max_distance(i)
      end
    end
    
    def calculate_max_distance(m) 
      return ((1-@minimum_similarity) * ([@text.length, m].min + @prefix_length))
    end
  end
end
