$: << File.dirname(__FILE__)
require File.dirname(__FILE__) + '/../../example_helper'
require 'ferret'
require 'content_generator'
require 'age_filter'
require 'benchmark'

ContentGenerator::WORDS.slice!(1000..ContentGenerator::WORDS.size)
DAY = 60*60*24
TODAY = Time.now.to_i/DAY
$index = Ferret::I.new(:default_field => 'content',
                       :dir => 'index')
INDEX_SIZE = 1_000_000

if ARGV[0] or $index.size < INDEX_SIZE
  $index = Ferret::Index::IndexWriter.new(:path => 'index', :create => true,
                                          :max_buffer_memory => 0x10000000)
  INDEX_SIZE.times do |i|
    puts i if i % 10_000 == 0
    $index << {
      :day => TODAY - rand(3000),
      :content => ContentGenerator.generate_text(100)
    }
  end
  $index = Ferret::I.new(:default_field => 'content',
                         :dir => 'index')
end

N = 10
SEARCH = ContentGenerator.generate_text(2)
Benchmark.bmbm do |x|
  x.report("no-filter") do
    N.times do
      $index.search(SEARCH).to_s(:day)
    end
  end
  x.report("c-extension") do
    N.times do
      age_filter = FerretExt::AgeFilter.new(TODAY)
      $index.search(SEARCH, :c_filter_proc => age_filter).to_s(:day)
    end
  end
  x.report("pure-ruby") do
    N.times do
      age_filter = lambda{|d, s, sea| 1.0/2**((TODAY - sea[d][:day].to_i)/50.0)}
      $index.search(SEARCH, :filter_proc => age_filter).to_s(:day)
    end
  end
  x.report("single-c-extension") do
    age_filter = FerretExt::AgeFilter.new(TODAY)
    N.times do
      $index.search(SEARCH, :c_filter_proc => age_filter).to_s(:day)
    end
  end
  x.report("single-pure-ruby") do
    age_filter = lambda{|d, s, sea| 1.0/2**((TODAY - sea[d][:day].to_i)/50.0)}
    N.times do
      $index.search(SEARCH, :filter_proc => age_filter).to_s(:day)
    end
  end
end
