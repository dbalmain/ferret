$: << File.dirname(__FILE__)
require File.dirname(__FILE__) + '/../../example_helper'
require 'ferret'
require 'content_generator'
require 'test/unit'
require 'age_filter'

class AgeFilterTest < Test::Unit::TestCase
  DAY = 60*60*24

  def setup
    @today = Time.now.to_i/DAY
    @index = Ferret::I.new(:default_field => 'content')
  end

  def do_proc_filter
    age_filter = lambda {|d, s, sea| 1.0/2**((@today - sea[d][:day].to_i)/50.0)}
    @index.search('dog day', :filter_proc => age_filter)
  end

  def do_extension_filter
    age_filter = FerretExt::AgeFilter.new(@today)
    @index.search('dog day', :c_filter_proc => age_filter)
  end

  def test_simple
    10000.times {
      @index << {:day => @today - rand(3000), :content => 'dog day'}
    }
    extension_results = do_extension_filter
    proc_results = do_proc_filter
    assert_equal(proc_results, extension_results)
    days = proc_results.hits.map {|h| @index[h.doc][:day].to_i}
    assert_equal(days, days.sort.reverse)
  end

  def test_generated_content
    500.times {
      @index << {
        :day => @today - rand(3000),
        :content => ContentGenerator.generate_text(1000)
      }
    }
    extension_results = do_extension_filter
    proc_results = do_proc_filter
    assert_equal(proc_results, extension_results)
    puts proc_results
  end
end
