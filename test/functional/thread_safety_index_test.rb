require File.dirname(__FILE__) + "/../test_helper"
require File.dirname(__FILE__) + "/../utils/number_to_spoken.rb"
require 'thread'

class IndexThreadSafetyTest < Test::Unit::TestCase
  include Ferret::Index
  include Ferret::Document

  INDEX_DIR = File.expand_path(File.join(File.dirname(__FILE__), "index"))
  ITERATIONS = 100
  NUM_THREADS = 10
  ANALYZER = Ferret::Analysis::Analyzer.new()

  def setup
    @index = Index.new(:path => 'index2',
                       :create => true,
                       :analyzer => ANALYZER,
                       :default_field => 'contents')
  end
   
  def indexing_thread()
    ITERATIONS.times do
      choice = rand()

      if choice > 0.98
        do_optimize
      elsif choice > 0.9
        do_delete_doc
      elsif choice > 0.7
        do_search
      else
        do_add_doc
      end
    end
  rescue => e
    puts e
    puts e.backtrace
    @index = nil
    raise e
  end 

  def do_optimize
    puts "Optimizing the index"
    @index.optimize
  end

  def do_delete_doc
    return if @index.size == 0
    doc_num = rand(@index.size)
    puts "Deleting #{doc_num} from index which has#{@index.has_deletions? ? "" : " no"} deletions"
    puts "document was already deleted" if (@index.deleted?(doc_num))
    @index.delete(doc_num)
  end

  def do_add_doc
    d = Document.new()
    n = rand(0xFFFFFFFF)
    d << Field.new("id", n.to_s, Field::Store::YES, Field::Index::UNTOKENIZED)
    d << Field.new("contents", n.to_spoken, Field::Store::NO, Field::Index::TOKENIZED)
    puts("Adding #{n}")
    @index << d
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
    NUM_THREADS.times do
      threads << Thread.new { indexing_thread }
    end

    threads.each {|t| t.join}
  end
end
