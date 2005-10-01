require File.dirname(__FILE__) + "/../../test_helper"
include Ferret::Analysis

class WordListLoaderTest < Test::Unit::TestCase
  def test_word_set_from_file()
    wl = WordListLoader.word_set_from_file(File.dirname(__FILE__) + '/data/wordfile')
    assert_equal(6, wl.size())
    assert(wl.member?('and'))
    assert(wl.member?('to'))
    assert(wl.member?('it'))
    assert(wl.member?('the'))
    assert(wl.member?('there'))
    assert(wl.member?('their'))
    assert(!wl.member?('horse'))
    assert(!wl.member?('judo'))
    assert(!wl.member?('dairy'))
  end

  def test_word_set_from_array()
    wl = WordListLoader.word_set_from_array(['and','to','it','the','there','their'])
    assert_equal(6, wl.size())
    assert(wl.member?('and'))
    assert(wl.member?('to'))
    assert(wl.member?('it'))
    assert(wl.member?('the'))
    assert(wl.member?('there'))
    assert(wl.member?('their'))
    assert(!wl.member?('horse'))
    assert(!wl.member?('judo'))
    assert(!wl.member?('dairy'))
  end
end
