require File.dirname(__FILE__) + "/../../test_helper"


class TermInfosIOTest < Test::Unit::TestCase
  include Ferret::Index

  DICT = [ "duad", "dual", "dualism", "dualist", "duality", "dualize", "duan",
      "duarchy", "dub", "dubber", "dubbin", "dubbing", "dubiety", "dubiosity",
      "dubious", "dubiously", "dubiousness", "dubitate", "dubitation", "dubnium",
      "dubonnet", "ducal", "ducat", "ducatoon", "duce", "duchess", "duchesse",
      "duchy", "duck", "duckbill", "duckboard", "ducker", "duckie", "ducking",
      "duckling", "duckpin", "duckshove", "duckshover", "ducktail", "duckwalk",
      "duckweed", "ducky", "duct", "ductile", "ductileness", "ductility",
      "ducting", "ductless", "ductule", "ductulus", "ductwork", "dud", "dudder",
      "duddery", "duddie", "duddy", "dude", "dudeen", "dudgeon", "due",
      "duecento", "duel", "dueler", "dueling", "duelist", "dueller", "duelling",
      "duellist", "duello", "duende", "dueness", "duenna", "duennaship", "duet",
      "duette", "duettino", "duettist", "duetto", "duff", "duffel", "duffer",
      "duffle", "dufus", "dug", "dugong", "dugout", "duiker", "duit", "duke",
      "dukedom", "dukeling", "dukery", "dukeship", "dulcamara", "dulcet",
      "dulcian", "dulciana", "dulcification", "dulcify", "dulcimer", "dulcimore",
      "dulcinea", "dulcitone", "dulcorate", "dule", "dulfer", "dulia", "dull",
      "dullard", "dullness", "dullsville", "dully", "dulness", "dulocracy",
      "dulosis", "dulse", "duly", "duma", "dumaist", "dumb", "dumbass",
      "dumbbell", "dumbcane", "dumbfound", "dumbfounder", "dumbhead",
      "dumbledore", "dumbly", "dumbness", "dumbo", "dumbstruck", "dumbwaiter",
      "dumdum", "dumfound", "dummerer", "dummkopf", "dummy", "dumortierite",
      "dump", "dumpbin", "dumpcart", "dumper", "dumpiness", "dumping",
      "dumpling", "dumplings", "dumpsite", "dumpster", "dumpy", "dun", "dunam",
      "dunce", "dunch", "dunder", "dunderhead", "dunderheadedness", "dunderpate",
      "dune", "duneland", "dunfish", "dung", "dungaree", "dungeon", "dungeoner",
      "dungheap", "dunghill", "dungy", "dunite", "duniwassal", "dunk", "dunker",
      "dunlin", "dunnage", "dunnakin", "dunness", "dunnite", "dunnock", "dunny",
      "dunt", "duo", "duodecillion", "duodecimal", "duodecimo", "duodenectomy",
      "duodenum", "duolog", "duologue", "duomo", "duopoly", "duopsony",
      "duotone", "dup", "dupability", "dupatta", "dupe", "duper", "dupery",
      "dupion", "duple", "duplet", "duplex", "duplexer", "duplexity",
      "duplicability", "duplicand", "duplicate", "duplication", "duplicator",
      "duplicature", "duplicitousness", "duplicity", "dupondius", "duppy",
      "dura", "durability", "durable", "durableness", "durably", "dural",
      "duralumin", "duramen", "durance", "duration", "durative", "durbar",
      "dure", "dures", "duress", "durgan", "durian", "durion", "durmast",
      "durn", "durned", "duro", "duroc", "durometer", "durr", "durra", "durrie",
      "durukuli", "durum", "durzi", "dusk", "duskiness", "dusky", "dust",
      "dustbin", "dustcart", "dustcloth", "dustcover", "duster", "dustheap",
      "dustiness", "dusting", "dustless", "dustman", "dustmop", "dustoff",
      "dustpan", "dustpanful", "dustrag", "dustsheet", "dustup", "dusty",
      "dutch", "dutchman", "duteous", "duteously", "duteousness", "dutiability",
      "dutiable", "dutifulness", "duty", "duumvir", "duumvirate", "duvet",
      "duvetine", "duvetyn", "duvetyne", "dux", "duyker"]

  TEST_SEGMENT = "_test"

  def setup()
    @dir = Ferret::Store::RAMDirectory.new
  end

  def tear_down()
    @dir.close()
  end

  def test_two_field_io
    term_dumbly = Term.new("word", "dumbly")
    term_dualize = Term.new("word", "dualize")
    term_rev_dualize = Term.new("reverse", "ezilaud")

    fis = FieldInfos.new
    fis.add("word", true, true)
    fis.add("reverse", true, true)
    terms = []
    term_infos = []
    tiw = TermInfosWriter.new(@dir, TEST_SEGMENT+"G", fis, 128)

    reverse_words = []
    DICT.each { |word| reverse_words << word.reverse }
    reverse_words.sort!
    reverse_words.each_with_index do |word, i|
      tiw.add(Term.new("reverse", word), TermInfo.new(1, i, i, 0))
    end
    DICT.each_with_index do |word, i|
      tiw.add(Term.new("word", word), TermInfo.new(1, 1000 + i, 1000 + i, 0))
    end

    tiw.close()
    tir = TermInfosReader.new(@dir, TEST_SEGMENT+"G", fis)
    assert_equal(564, tir.size)
    assert_equal(16, tir.skip_interval)
    assert_equal(561, tir.get_terms_position(Term.new("word", "duvetyne")))
    assert_equal(TermInfo.new(1, 1005, 1005, 0), tir.get_term_info(term_dualize))
    assert_equal(TermInfo.new(1, 70, 70, 0), tir.get_term_info(term_rev_dualize))
  end

  def test_io
    term_dumbly = Term.new("word", "dumbly")
    term_dualize = Term.new("word", "dualize")

    fis = FieldInfos.new
    fis.add("word", true, true)
    terms = []
    term_infos = []
    tiw = TermInfosWriter.new(@dir, TEST_SEGMENT, fis, 128)
    DICT.each_with_index do |word, i|
      terms << Term.new("word", word)
      term_infos << TermInfo.new(1, i, i, 0)
      tiw.add(terms[i], term_infos[i])
    end
    tiw.close()
    tir = TermInfosReader.new(@dir, TEST_SEGMENT, fis)
    assert_equal(282, tir.size)
    assert_equal(16, tir.skip_interval)
    assert_equal(281, tir.get_terms_position(Term.new("word", "duyker")))
    assert_equal(279, tir.get_terms_position(Term.new("word", "duvetyne")))
    assert_equal(254, tir.get_terms_position(Term.new("word", "dusting")))
    assert_equal(255, tir.get_terms_position(Term.new("word", "dustless")))
    assert_equal(256, tir.get_terms_position(Term.new("word", "dustman")))
    assert_equal(257, tir.get_terms_position(Term.new("word", "dustmop")))
    assert_equal(TermInfo.new(1, 5, 5, 0), tir.get_term_info(term_dualize))
    assert_equal(term_dumbly, tir.get_term(127))
    terms = tir.terms_from(term_dumbly)
    assert_equal(term_dumbly, terms.term)
    assert(terms.next?)
    assert_equal(Term.new("word", "dumbness"), terms.term)
    assert(terms.next?)
    assert_equal(Term.new("word", "dumbo"), terms.term)
  end

  def test_small_writer
    fis = FieldInfos.new
    fis.add("author", true, true)
    fis.add("title", true, true)
    tiw = TermInfosWriter.new(@dir, TEST_SEGMENT, fis, 128)
    terms = [ Term.new("author", "Martel"),
              Term.new("title", "Life of Pi"),
              Term.new("author", "Martin"),
              Term.new("title", "Life on the edge") ].sort
    term_infos = []
    4.times {|i| term_infos << TermInfo.new(i,i,i,i)}
    4.times {|i| tiw.add(terms[i], term_infos[i]) }
    tiw.close()

    tis_file = @dir.open_input(TEST_SEGMENT + ".tis")
    tii_file = @dir.open_input(TEST_SEGMENT + ".tii")
    assert_equal(TermInfosWriter::FORMAT, tis_file.read_int())
    assert_equal(4, tis_file.read_long())  # term count
    assert_equal(128, tis_file.read_int()) # @index_interval
    assert_equal(16, tis_file.read_int())  # @skip_interval
    assert_equal(0, tis_file.read_vint())  # string_equal length
    assert_equal(6, tis_file.read_vint())  # rest of string length
    tis_file.read_chars(author = "", 0, 6) # the difference string
    assert_equal("Martel", author.to_s)
    assert_equal(0, tis_file.read_vint())  # field number
    assert_equal(0, tis_file.read_vint())  # doc_freq
    assert_equal(0, tis_file.read_vlong()) # freq pointer difference
    assert_equal(0, tis_file.read_vlong()) # prox pointer difference
    assert_equal(4, tis_file.read_vint())  # string_equal length
    assert_equal(2, tis_file.read_vint())  # rest of string length
    tis_file.read_chars(author = "", 0, 2) # the difference string
    assert_equal("in", author.to_s)                                  
    assert_equal(0, tis_file.read_vint())  # field number
    assert_equal(1, tis_file.read_vint())  # doc_freq
    assert_equal(1, tis_file.read_vlong()) # freq pointer difference
    assert_equal(1, tis_file.read_vlong()) # prox pointer difference
    assert_equal(0, tis_file.read_vint())  # string_equal length
    assert_equal(10, tis_file.read_vint()) # rest of string length
    tis_file.read_chars(title = "", 0, 10) # the difference string
    assert_equal("Life of Pi", title.to_s)                           
    assert_equal(1, tis_file.read_vint())  # field number
    assert_equal(2, tis_file.read_vint())  # doc_freq
    assert_equal(1, tis_file.read_vlong()) # freq pointer difference
    assert_equal(1, tis_file.read_vlong()) # prox pointer difference
    assert_equal(6, tis_file.read_vint())  # string_equal length
    assert_equal(10, tis_file.read_vint()) # rest of string length
    tis_file.read_chars(title = "", 0, 10) # the difference string
    assert_equal("n the edge", title.to_s)                           
    assert_equal(1, tis_file.read_vint())  # field number
    assert_equal(3, tis_file.read_vint())  # doc_freq
    assert_equal(1, tis_file.read_vlong()) # freq pointer difference
    assert_equal(1, tis_file.read_vlong()) # prox pointer difference

    assert_equal(TermInfosWriter::FORMAT, tii_file.read_int())
    assert_equal(1, tii_file.read_long())
    assert_equal(128, tii_file.read_int())
    assert_equal(16, tii_file.read_int())
    assert_equal(0, tii_file.read_vint())  # string_equal length
    assert_equal(0, tii_file.read_vint())  # rest of string length
    assert_equal(0xFFFFFFFF, tii_file.read_vint())  # field number
    assert_equal(0, tii_file.read_vint())  # doc_freq
    assert_equal(0, tii_file.read_vlong()) # freq pointer difference
    assert_equal(0, tii_file.read_vlong()) # prox pointer difference
    assert_equal(20, tii_file.read_vlong()) # pointer to first element in other
  end
end
