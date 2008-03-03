#require 'rubygems'
$: << '../../../lib'
$: << '../../../ext'
require 'ferret'
require 'age_filter'

DAY = 60*60*24
WORDS = %w{one two three four five six seven eight nine ten}
def random_sentence
  Array.new((5 + rand(5))) {WORDS[rand(WORDS.size)]}.join(' ')
end

today = Time.now.to_i/DAY
index = Ferret::I.new(:default_field => 'content')
10000.times {
  index << {:day => today - rand(3000), :content => random_sentence}
  #index << {:day => today - rand(3000), :content => 'one two three four'}
}

prc = lambda {|d, s, sea| x = 1.0/2**((Time.now.to_i/DAY-sea[d][:day].to_i)/50.0); puts x; x}

age_filter = FerretExt::AgeFilter.new(Time.now.to_i/DAY)
index.search_each('+one +two +three +four', :c_filter_proc => age_filter) do |d,s|
#index.search_each('+one +two +three +four') do |d,s|
#index.search_each('+one +two +three +four', :filter_proc => prc) do |d,s|
  puts "(doc: %4d, score: %1.3f) - day:%6s, content: %s" % [d, s, index[d][:day], index[d][:content]]
end
