require File.dirname(__FILE__) + "/../../test_helper"


class IndexTest < Test::Unit::TestCase
  include Ferret::Index
  include Ferret::Analysis
  include Ferret::Store

  def setup()
    @qp = Ferret::QueryParser.new()
  end

  def tear_down()
  end

  def check_results(index, query, expected)
    cnt = 0
    index.search_each(query) do |doc, score|
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
      {"def_field" => "two", :field2 => "three multi2", "field3" => "five multi"}
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
    index = Index.new(:default_field => "def_field")
    do_test_index_with_hash(index)
    index = Index.new(:default_field => "def_field")
    do_test_index_with_doc_array(index)
  end

  def test_fs_index
    fs_path = File.expand_path(File.join(File.dirname(__FILE__), '../../temp/fsdir'))
    `rm -rf #{File.join(fs_path, "*")}`
    assert_raise(Errno::ENOENT) {Index.new(:path => fs_path, :create_if_missing => false, :default_field => "def_field")}
    index = Index.new(:path => fs_path, :default_field => "def_field")
    do_test_index_with_array(index)
    `rm -rf #{File.join(fs_path, "*")}`
    index = Index.new(:path => fs_path, :create => true, :default_field => "def_field")
    do_test_index_with_hash(index)
    index = Index.new(:path => fs_path, :create => true, :default_field => "def_field")
    do_test_index_with_doc_array(index)
  end

  def test_fs_index_is_persistant
    fs_path = File.expand_path(File.join(File.dirname(__FILE__), '../../temp/fsdir'))
    `rm -rf #{File.join(fs_path, "*")}`
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
    index.persist(fs_path)
    assert_equal(3, index.size)
    assert_equal("zero", index.doc[0]["f"])
    index.close

    index = Index.new(:path => fs_dir)
    assert_equal(3, index.size)
    assert_equal("zero", index.doc[0]["f"])
  end
end
