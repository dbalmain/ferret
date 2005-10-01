require 'set'
module Ferret
  module Analysis
    # Loader for text files that represent a list of stopwords.
    module WordListLoader
      # Loads a text file and adds every line as an entry to a HashSet (omitting
      # leading and trailing whitespace). Every line of the file should contain only 
      # one word. The words need to be in lowercase if you make use of an
      # Analyzer which uses LowerCaseFilter (like GermanAnalyzer).
      # 
      # path:: path to file containing the wordlist
      # Return:: A HashSet with the file's words
      def WordListLoader.word_set_from_file(path)
        result = Set.new()
        File.open(path) do |word_file|
          # we have to strip the end of line characters
          word_file.each {|line| result << line[0..-2] }
        end
        return result
      end

      def WordListLoader.word_set_from_array(word_array)
        result = Set.new()
        word_array.each {|word| result << word }
        return result
      end
    end
  end
end
