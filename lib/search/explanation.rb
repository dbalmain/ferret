module Ferret::Search
  # Expert: Describes the score computation for document and query. 
  class Explanation
    attr_accessor :value, :description, :details

    def initialize(value = nil, description = nil) 
      @value = value
      @description = description
      @details = []
    end

    def <<(detail)
      @details << detail
    end

    # Render an explanation as text. 
    def to_s(depth = 0) 
      buffer = "  " * depth
      buffer << "#{@value} = #{@description}\n"

      @details.each do |detail|
        buffer << detail.to_s(depth + 1)
      end
      return buffer
    end

    # Render an explanation as HTML. 
    def to_html() 
      buffer = "<ul>\n"
      buffer << "<li>#{@value} = #{@description}</li>\n"

      @details.each do |detail|
        buffer << detail.to_html
      end

      buffer << "</ul>\n"

      return buffer
    end
  end
end
