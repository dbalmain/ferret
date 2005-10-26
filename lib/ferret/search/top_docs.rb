module Ferret::Search
  # Expert: Returned by low-level search implementations.
  # See Searcher#search
  class TopDocs
    # Expert: The total number of hits for the query.
    # See Hits#length()
    attr_accessor :score_docs, :total_hits, :fields
    alias :size :total_hits

    # iterate through each of the score docs, yielding the document number and
    # the score. eg:
    #
    #   top_docs.each do |doc, score|
    #     puts "Doc number #{doc} found with score of #{score}"}
    #   end
    #
    def each
      score_docs.each {|sd| yield(sd.doc, sd.score) }
    end

    # Expert: Constructs a TopDocs.
    def initialize(total_hits, score_docs, fields = SortField::FIELD_SCORE) 
      @total_hits = total_hits
      @score_docs = score_docs
      @fields = fields
    end

    def to_s
      buffer = "#{total_hits} hits sorted by <"
      buffer << [fields].flatten.map {|field| "#{@field}" }.join(", ")
      buffer << ">:\n"
      score_docs.each {|sd| buffer << "\t#{sd}\n" }
      return buffer
    end
  end
end
