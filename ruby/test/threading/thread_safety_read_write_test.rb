require File.dirname(__FILE__) + "/../test_helper"
require File.dirname(__FILE__) + "/number_to_spoken.rb"
require 'thread'

class IndexThreadSafetyReadWriteTest < Test::Unit::TestCase
  include Ferret::Index

  INDEX_DIR = File.expand_path(File.join(File.dirname(__FILE__), "index"))
  ITERATIONS = 10000
  ANALYZER = Ferret::Analysis::Analyzer.new()

  def setup
    @index = Index.new(:path => INDEX_DIR,
                       :create => true,
                       :analyzer => ANALYZER,
                       :default_field => :content)
  end

  def search_thread()
    ITERATIONS.times do
      do_search()
      sleep(rand(1))
    end
  rescue => e
    puts e
    puts e.backtrace
    @index = nil
    raise e
  end 

  def index_thread()
    ITERATIONS.times do
      do_add_doc()
      sleep(rand(1))
    end
  rescue => e
    puts e
    puts e.backtrace
    @index = nil
    raise e
  end 

  def do_add_doc
    n = rand(0xFFFFFFFF)
    d = {:id => n.to_s, :content => n.to_spoken}
    puts("Adding #{n}")
    begin
      @index << d
    rescue => e
      puts e
      puts e.backtrace
      @index = nil
      raise e
    end
  end
  
  def do_search
    n = rand(0xFFFFFFFF)
    puts("Searching for #{n}")
    hits = @index.search_each(n.to_spoken, :num_docs => 3) do |d, s|
      puts "Hit for #{n}: #{@index[d]["id"]} - #{s}"
    end
    puts("Searched for #{n}: total = #{hits}")
  end

  def test_threading
    threads = []
    threads << Thread.new { search_thread }
    threads << Thread.new { index_thread }

    threads.each { |t| t.join }
  end
end
