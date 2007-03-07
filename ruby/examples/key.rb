require 'rubygems'
require 'ferret'

index = Ferret::I.new(:key => :id)

[
  {:id => '1', :text => 'one'},
  {:id => '2', :text => 'Two'},
  {:id => '3', :text => 'Three'},
  {:id => '1', :text => 'One'}
].each {|doc| index << doc}

puts index.size                       # => 3
puts index['1'].load.inspect          # => {:text=>"One", :id=>"1"}
puts index.search('id:1').to_s(:text)
    # => TopDocs: total_hits = 1, max_score = 1.287682 [
    #            3 "One": 1.287682
    #    ]

