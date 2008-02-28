require 'rubygems'
require 'ferret'
require 'age_filter'

DAY = 60*60*24
WORDS = %w{one two three four five size seven eight nine ten}
def random_sentence
  Array.new((5 + rand(5))) {WORDS[rand(WORDS.size)]}.join(' ')
end

today = Time.now.to_i/DAY
index = Ferret::I.new(:default_field => 'content')
100.times {
  index << {:day => today - rand(3000), :content => random_sentence}
}

age_filter = FerretExt::AgeFilter.new(Time.now.to_i/DAY)
puts index.search('+one +two +three +four', :c_filter_proc => age_filter)
