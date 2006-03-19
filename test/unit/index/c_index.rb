require File.dirname(__FILE__) + "/../../test_helper"


class IndexTest < Test::Unit::TestCase
  include Ferret::Index
  include Ferret::Search
  include Ferret::Analysis
  include Ferret::Store
  include Ferret::Document

  def setup()
  end

  def tear_down()
  end

  def check_results(index, query, expected)
    cnt = 0
    #puts "#{query} - #{expected.inspect}"
    #puts index.size
    index.search_each(query) do |doc, score|
      #puts "doc-#{doc} score=#{score}"
      assert(expected.index(doc))
      cnt += 1
    end
    assert_equal(expected.length, cnt)
  end

  def do_test_index_with_array(index)
    data = [
      ["one two"],
      ["one", "three"],
      ["two"],
      ["one", "four"],
      ["one two"],
      ["two", "three", "four"],
      ["one"],
      ["two", "three", "four", "five"]
    ]
    data.each {|doc| index << doc }
    assert_equal(8, index.size)
    q = "one"
    check_results(index, q, [0, 1, 3, 4, 6])
    q = "one AND two"
    check_results(index, q, [0, 4])
    q = "one OR five"
    check_results(index, q, [0, 1, 3, 4, 6, 7])
    assert_equal("two three four five", index.doc(7)["def_field"])
  end

  def do_test_index_with_hash(index)
    data = [
      {"def_field" => "one two"},
      {"def_field" => "one", "field2" => "three"},
      {"def_field" => "two"},
      {"def_field" => "one", "field2" => "four"},
      {"def_field" => "one two"},
      {"def_field" => "two", "field2" => "three", "field3" => "four"},
      {"def_field" => "one"},
      {"def_field" => "two", "field2" => "three", "field3" => "five"}
    ]
    data.each {|doc| index << doc }
    q = "one AND two"
    check_results(index, q, [0, 4])
    q = "one OR five"
    check_results(index, q, [0, 1, 3, 4, 6])
    q = "one OR field3:five"
    check_results(index, q, [0, 1, 3, 4, 6, 7])
    assert_equal("four", index[5]["field3"])
    q = "field3:f*"
    check_results(index, q, [5, 7])
    q = "two AND field3:f*"
    check_results(index, q, [5, 7])
    assert_equal("five", index.doc(7)["field3"])
    assert_equal("two", index.doc(7)["def_field"])
  end

  def do_test_index_with_doc_array(index)
    data = [
      {"def_field" => "one two multi", :id => "me"},
      {"def_field" => "one", :field2 => "three multi"},
      {"def_field" => "two"},
      {"def_field" => "one", :field2 => "four"},
      {"def_field" => "one two"},
      {"def_field" => "two", :field2 => "three", "field3" => "four"},
      {"def_field" => "one multi2"},
      {"def_field" => "two", :field2 => "this three multi2", "field3" => "five multi"}
    ]
    data.each {|doc| index << doc }
    q = "one AND two"
    check_results(index, q, [0, 4])
    q = "one OR five"
    check_results(index, q, [0, 1, 3, 4, 6])
    q = "one OR field3:five"
    check_results(index, q, [0, 1, 3, 4, 6, 7])
    q = "two AND (field3:f*)"
    check_results(index, q, [5, 7])
    q = "*:(multi OR multi2)"
    check_results(index, q, [0, 1, 6, 7])
    q = "field2|field3:(multi OR multi2)"
    check_results(index, q, [1, 7])
    doc = index[5]
    assert_equal("three", index[5]["field2"])
    assert(!index.has_deletions?)
    assert(!index.deleted?(5))
    assert_equal(8, index.size)
    index.delete(5)
    assert(index.has_deletions?)
    assert(index.deleted?(5))
    assert_equal(7, index.size)
    q = "two AND (field3:f*)"
    check_results(index, q, [7])
    doc["field2"] = "dave"
    index << doc
    check_results(index, q, [6, 7])
    check_results(index, "*:this", [])
    assert_equal(8, index.size)
    assert_equal("dave", index[7]["field2"])
    index.optimize
    check_results(index, q, [6, 7])
    t = Term.new("field2", "three")
    index.delete(t)
    assert(index.deleted?(1))
    assert(index.deleted?(6))
    assert(! index.deleted?(7))
    t = Term.new("field2", "four")
    assert_equal("one", index[t]["def_field"])
    assert_equal("one two multi", index["me"]["def_field"])
    index.delete("me")
    assert(index.deleted?(0))
  end

  def test_ram_index
    index = Index.new(:default_field => "def_field")
    do_test_index_with_array(index)
    index.close

    index = Index.new(:default_field => "def_field")
    do_test_index_with_hash(index)
    index.close

    index = Index.new(:default_field => "def_field", :id_field => "id")
    do_test_index_with_doc_array(index)
    index.close
  end

  def test_fs_index
    fs_path = File.expand_path(File.join(File.dirname(__FILE__), '../../temp/fsdir'))
    Dir[File.join(fs_path, "*")].each {|path| begin File.delete(path) rescue nil end}
    assert_raise(StandardError) do
      Index.new(:path => fs_path,
                :create_if_missing => false,
                :default_field => "def_field")
    end
    index = Index.new(:path => fs_path, :default_field => "def_field")
    do_test_index_with_array(index)
    index.close

    Dir[File.join(fs_path, "*")].each {|path| begin File.delete(path) rescue nil end}
    index = Index.new(:path => fs_path, :default_field => "def_field")
    do_test_index_with_hash(index)
    index.close

    Dir[File.join(fs_path, "*")].each {|path| begin File.delete(path) rescue nil end}
    index = Index.new(:path => fs_path,
                      :default_field => "def_field",
                      :id_field => "id")
    do_test_index_with_doc_array(index)
    index.close
  end

  def test_fs_index_is_persistant
    fs_path = File.expand_path(File.join(File.dirname(__FILE__), '../../temp/fsdir'))
    Dir[File.join(fs_path, "*")].each {|path| begin File.delete(path) rescue nil end}
    data = [
      {"def_field" => "one two", :id => "me"},
      {"def_field" => "one", :field2 => "three"},
      {"def_field" => "two"},
      {"def_field" => "one", :field2 => "four"},
      {"def_field" => "one two"},
      {"def_field" => "two", :field2 => "three", "field3" => "four"},
      {"def_field" => "one"},
      {"def_field" => "two", :field2 => "three", "field3" => "five"}
    ]
    index = Index.new(:path => fs_path, :default_field => "def_field")
    data.each {|doc| index << doc }
    assert_equal(8, index.size)
    index.close

    index = Index.new(:path => fs_path, :create_if_missing => false)
    assert_equal(8, index.size)
    assert_equal("four", index[5]["field3"])
    index.close
  end

  def test_merging_indexes
    data = [
      {"f" => "zero"},
      {"f" => "one"},
      {"f" => "two"}
    ]
    index1 = Index.new(:default_field => "f")
    data.each {|doc| index1 << doc }
    data = [
      {"f" => "three"},
      {"f" => "four"},
      {"f" => "five"}
    ]
    index2 = Index.new(:default_field => "f")
    data.each {|doc| index2 << doc }
    data = [
      {"f" => "six"},
      {"f" => "seven"},
      {"f" => "eight"}
    ]
    index3 = Index.new(:default_field => "f")
    data.each {|doc| index3 << doc }

    index = Index.new(:default_field => "f")
    index.add_indexes(index1)
    assert_equal(3, index.size)
    assert_equal("zero", index[0]["f"])
    index.add_indexes([index2, index3])
    assert_equal(9, index.size)
    assert_equal("zero", index[0]["f"])
    assert_equal("eight", index[8]["f"])
    index1.close
    index2.close
    index3.close
    assert_equal("seven", index[7]["f"])
    data = [
      {"f" => "alpha"},
      {"f" => "beta"},
      {"f" => "charlie"}
    ]
    dir1 = RAMDirectory.new
    index1 = Index.new(:dir => dir1, :default_field => "f")
    data.each {|doc| index1 << doc }
    index1.flush
    data = [
      {"f" => "delta"},
      {"f" => "echo"},
      {"f" => "foxtrot"}
    ]
    dir2 = RAMDirectory.new
    index2 = Index.new(:dir => dir2, :default_field => "f")
    data.each {|doc| index2 << doc }
    index2.flush
    data = [
      {"f" => "golf"},
      {"f" => "india"},
      {"f" => "juliet"}
    ]
    dir3 = RAMDirectory.new
    index3 = Index.new(:dir => dir3, :default_field => "f")
    data.each {|doc| index3 << doc }
    index3.flush

    index.add_indexes(dir1)
    assert_equal(12, index.size)
    assert_equal("alpha", index[9]["f"])
    index.add_indexes([dir2, dir3])
    assert_equal(18, index.size)
    assert_equal("juliet", index[17]["f"])
    index1.close
    dir1.close
    index2.close
    dir2.close
    index3.close
    dir3.close
    assert_equal("golf", index[15]["f"])
    index.close
  end

  def test_persist_index
    data = [
      {"f" => "zero"},
      {"f" => "one"},
      {"f" => "two"}
    ]
    index = Index.new(:default_field => "f")
    data.each {|doc| index << doc }
    fs_path = File.expand_path(File.join(File.dirname(__FILE__), '../../temp/fsdir'))
    index.persist(fs_path, true)
    assert_equal(3, index.size)
    assert_equal("zero", index[0]["f"])
    index.close

    index = Index.new(:path => fs_path)
    assert_equal(3, index.size)
    assert_equal("zero", index[0]["f"])
    index.close


    data = [
      {"f" => "romeo"},
      {"f" => "sierra"},
      {"f" => "tango"}
    ]
    index = Index.new(:default_field => "f")
    data.each {|doc| index << doc }
    assert_equal(3, index.size)
    assert_equal("romeo", index[0]["f"])
    dir = FSDirectory.new(fs_path, false)
    index.persist(dir)
    assert_equal(6, index.size)
    assert_equal("zero", index[0]["f"])
    assert_equal("romeo", index[3]["f"])
    index.close

    index = Index.new(:path => fs_path)
    assert_equal(6, index.size)
    assert_equal("zero", index[0]["f"])
    assert_equal("romeo", index[3]["f"])
    index.close
  end

  def test_auto_update_when_externally_modified()
    fs_path = File.expand_path(File.join(File.dirname(__FILE__), '../../temp/fsdir'))
    index = Index.new(:path => fs_path, :default_field => "f", :create => true)
    index << "document 1"
    assert_equal(1, index.size)

    index2 = Index.new(:path => fs_path, :default_field => "f")
    assert_equal(1, index2.size)
    index2 << "document 2"
    assert_equal(2, index2.size)
    assert_equal(2, index.size)
    top_docs = index.search("content3")
    assert_equal(0, top_docs.size)

    iw = IndexWriter.new(fs_path, :analyzer => WhiteSpaceAnalyzer.new())
    doc = Document.new
    doc << Field.new("f", "content3", Field::Store::YES, Field::Index::TOKENIZED)
    iw << doc
    iw.close()
    top_docs = index.search("content3")
    assert_equal(1, top_docs.size)
    assert_equal(3, index.size)
    assert_equal("content3", index[2]["f"])
    index.close
  end

  def test_delete
    data = [
      {:id => 0, :cat => "/cat1/subcat1"},
      {:id => 1, :cat => "/cat1/subcat2"},
      {:id => 2, :cat => "/cat1/subcat2"},
      {:id => 3, :cat => "/cat1/subcat3"},
      {:id => 4, :cat => "/cat1/subcat4"},
      {:id => 5, :cat => "/cat2/subcat1"},
      {:id => 6, :cat => "/cat2/subcat2"},
      {:id => 7, :cat => "/cat2/subcat3"},
      {:id => 8, :cat => "/cat2/subcat4"},
      {:id => 9, :cat => "/cat2/subcat5"},
    ]
    index = Index.new(:analyzer => WhiteSpaceAnalyzer.new)
    data.each {|doc| index << doc }
    assert_equal(10, index.size)
    assert_equal(1, index.search("id:9").size)
    index.delete(9)
    assert_equal(9, index.size)
    assert_equal(0, index.search("id:9").size)
    assert_equal(1, index.search("id:8").size)
    index.delete("8")
    assert_equal(8, index.size)
    assert_equal(0, index.search("id:8").size)
    assert_equal(5, index.search("cat:/cat1*").size)
    index.query_delete("cat:/cat1*")
    assert_equal(3, index.size)
    assert_equal(0, index.search("cat:/cat1*").size)
    index.close
  end

  def test_update
    data = [
      {:id => 0, :cat => "/cat1/subcat1", :content => "content0"},
      {:id => 1, :cat => "/cat1/subcat2", :content => "content1"},
      {:id => 2, :cat => "/cat1/subcat2", :content => "content2"},
      {:id => 3, :cat => "/cat1/subcat3", :content => "content3"},
      {:id => 4, :cat => "/cat1/subcat4", :content => "content4"},
      {:id => 5, :cat => "/cat2/subcat1", :content => "content5"},
      {:id => 6, :cat => "/cat2/subcat2", :content => "content6"},
      {:id => 7, :cat => "/cat2/subcat3", :content => "content7"},
      {:id => 8, :cat => "/cat2/subcat4", :content => "content8"},
      {:id => 9, :cat => "/cat2/subcat5", :content => "content9"},
    ]
    index = Index.new(:analyzer => WhiteSpaceAnalyzer.new,
                      :default_field => :content,
                      :id_field => :id)
    data.each { |doc| index << doc }
    assert_equal(10, index.size)
    assert_equal("content5", index["5"][:content])
    index.update(5, "content five")
    assert_equal("content five", index["5"][:content])
    assert_equal(nil, index["5"][:extra_content])
    index.update("5", {:cat => "/cat1/subcat6",
                       :content => "high five",
                       :extra_content => "hello"})
    assert_equal("hello", index["5"][:extra_content])
    assert_equal("high five", index["5"][:content])
    assert_equal("/cat1/subcat6", index["5"][:cat])
    assert_equal("content9", index["9"][:content])
    index.update(Term.new("content", "content9"), {:content => "content nine"})
    assert_equal("content nine", index["9"][:content])
    assert_equal("content0", index["0"][:content])
    assert_equal(nil, index["0"][:extra_content])
    document = index[0]
    document[:content] = "content zero"
    document[:extra_content] = "extra content"
    index.update(0, document)
    assert_equal("content zero", index["0"][:content])
    assert_equal("extra content", index["0"][:extra_content])
    assert_equal(nil, index["1"][:tag])
    assert_equal(nil, index["2"][:tag])
    assert_equal(nil, index["3"][:tag])
    assert_equal(nil, index["4"][:tag])
    index.query_update("id:<5 AND cat:>=/cat1/subcat2", {:tag => "cool"})
    assert_equal("cool", index["1"][:tag])
    assert_equal("cool", index["2"][:tag])
    assert_equal("cool", index["3"][:tag])
    assert_equal("cool", index["4"][:tag])
    assert_equal(4, index.search("tag:cool").size)
    index.close
  end

  def test_index_key
    data = [
      {:id => 0, :val => "one"},
      {:id => 0, :val => "two"},
      {:id => 1, :val => "three"},
      {:id => 1, :val => "four"},
    ]
    index = Index.new(:analyzer => WhiteSpaceAnalyzer.new,
                      :key => "id")
    data.each { |doc| index << doc }
    assert_equal(2, index.size)
    assert_equal("two", index[0][:val])
    assert_equal("four", index[1][:val])
    index.close
  end

  def test_index_multi_key
    data = [
      {:id => 0, :table => "product", :product => "tent"},
      {:id => 0, :table => "location", :location => "first floor"},
      {:id => 0, :table => "product", :product => "super tent"},
      {:id => 0, :table => "location", :location => "second floor"},
      {:id => 1, :table => "product", :product => "backback"},
      {:id => 1, :table => "location", :location => "second floor"},
      {:id => 1, :table => "location", :location => "first floor"},
      {:id => 1, :table => "product", :product => "rucksack"},
      {:id => 1, :table => "product", :product => "backpack"}
    ]
    index = Index.new(:analyzer => WhiteSpaceAnalyzer.new,
                      :key => ["id", "table"])
    data.each { |doc| index << doc }
    assert_equal(4, index.size)
    assert_equal("super tent", index[0][:product])
    assert_equal("second floor", index[1][:location])
    assert_equal("backpack", index[3][:product])
    assert_equal("first floor", index[2][:location])
    index.close
  end

  def test_index_multi_key_untokenized
    data = [
      {:id => 0, :table => "Product", :product => "tent"},
      {:id => 0, :table => "location", :location => "first floor"},
      {:id => 0, :table => "Product", :product => "super tent"},
      {:id => 0, :table => "location", :location => "second floor"},
      {:id => 1, :table => "Product", :product => "backback"},
      {:id => 1, :table => "location", :location => "second floor"},
      {:id => 1, :table => "location", :location => "first floor"},
      {:id => 1, :table => "Product", :product => "rucksack"},
      {:id => 1, :table => "Product", :product => "backpack"}
    ]
    index = Index.new(:analyzer => Analyzer.new,
                      :key => ["id", "table"])
    data.each do |dat|
      doc = Document.new
      dat.each_pair do |key, value|
        if ([:id, :table].include?(key))
          doc << Field.new(key, value, Field::Store::YES, Field::Index::UNTOKENIZED)
        else
          doc << Field.new(key, value, Field::Store::YES, Field::Index::TOKENIZED)
        end
      end
      index << doc 
    end
    assert_equal(4, index.size)
    assert_equal("super tent", index[0][:product])
    assert_equal("second floor", index[1][:location])
    assert_equal("backpack", index[3][:product])
    assert_equal("first floor", index[2][:location])
    index.close
  end

  def test_sortby_date
    data = [
      {:content => "one", :date => "20051023"},
      {:content => "two", :date => "19530315"},
      {:content => "three four", :date => "19390912"},
      {:content => "one", :date => "19770905"},
      {:content => "two", :date => "19810831"},
      {:content => "three", :date => "19790531"},
      {:content => "one", :date => "19770725"},
      {:content => "two", :date => "19751226"},
      {:content => "four", :date => "19390912"}
    ]
    index = Index.new(:analyzer => WhiteSpaceAnalyzer.new)
    data.each { |doc|
      document = Document.new
      doc.each_pair do |key, value|
        document << Field.new(key.to_s, value, Field::Store::YES, Field::Index::TOKENIZED)
      end
      index << document
    }
    sf_date = SortField.new("date", {:sort_type => SortField::SortType::INTEGER})
    #top_docs = index.search("one", :sort => [sf_date, SortField::FIELD_SCORE])
    top_docs = index.search("one", :sort => Sort.new("date"))
    assert_equal(3, top_docs.size)
    assert_equal("19770725", index[top_docs.score_docs[0].doc][:date])
    assert_equal("19770905", index[top_docs.score_docs[1].doc][:date])
    assert_equal("20051023", index[top_docs.score_docs[2].doc][:date])
    top_docs = index.search("one two three four",
                            :sort => [sf_date, SortField::FIELD_SCORE])
    assert_equal("19390912", index[top_docs.score_docs[0].doc][:date])
    assert_equal("three four", index[top_docs.score_docs[0].doc][:content])
    assert_equal("19390912", index[top_docs.score_docs[1].doc][:date])
    assert_equal("four", index[top_docs.score_docs[1].doc][:content])
    assert_equal("19530315", index[top_docs.score_docs[2].doc][:date])

    top_docs = index.search("one two three four",
                            :sort => [:date, :content])
    assert_equal("19390912", index[top_docs.score_docs[0].doc][:date])
    assert_equal("four", index[top_docs.score_docs[0].doc][:content])
    assert_equal("19390912", index[top_docs.score_docs[1].doc][:date])
    assert_equal("three four", index[top_docs.score_docs[1].doc][:content])
    assert_equal("19530315", index[top_docs.score_docs[2].doc][:date])

    index.close
  end

  def test_auto_flush
    fs_path = File.expand_path(File.join(File.dirname(__FILE__), '../../temp/fsdir'))
    Dir[File.join(fs_path, "*")].each {|path| begin File.delete(path) rescue nil end}
    data = %q(one two three four five six seven eight nine ten eleven twelve)
    index1 = Index.new(:path => fs_path, :auto_flush => true)
    index2 = Index.new(:path => fs_path, :auto_flush => true)
    begin
      data.each do |doc|
        index1 << doc
        index2 << doc
      end
      5.times do |i|
        index1.delete(i)
        index2.delete(i + 5)
      end
      index1.optimize
      index2 << "thirteen"
    rescue Exception => e
      assert(false, "This should not cause an error when auto flush has been set")
    end
    index1.close
    index2.close
  end
end
