require File.dirname(__FILE__) + "/../../test_helper"

include Ferret::Document
include Ferret::Index

module IndexTestHelper
  def IndexTestHelper.make_binary(size)
    tmp = Array.new(size)
    size.times {|i| tmp[i] = i%256 }
    return tmp.pack("c*")
  end

  BINARY_DATA = IndexTestHelper.make_binary(256)
  COMPRESSED_BINARY_DATA = IndexTestHelper.make_binary(56)

  def IndexTestHelper.prepare_document
    doc = Document::Document.new()

    doc << Field.new("text_field1", "field one text", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::NO)
    doc << Field.new("text_field2", "field field field two text", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    doc << Field.new("key_field", "keyword", Field::Store::YES, Field::Index::UNTOKENIZED)
    doc << Field.new("unindexed_field", "unindexed field text", Field::Store::YES, Field::Index::NO)
    doc << Field.new("unstored_field1", "unstored field text one", Field::Store::NO, Field::Index::TOKENIZED, Field::TermVector::NO)
    doc << Field.new("unstored_field2", "unstored field text two", Field::Store::NO, Field::Index::TOKENIZED, Field::TermVector::YES)
    doc << Field.new("compressed_field", "compressed text", Field::Store::COMPRESS, Field::Index::TOKENIZED, Field::TermVector::YES)
    doc << Field.new_binary_field("binary_field", BINARY_DATA, Field::Store::YES)
    doc << Field.new_binary_field("compressed_binary_field", COMPRESSED_BINARY_DATA, Field::Store::COMPRESS)
    return doc
  end

  def IndexTestHelper.prepare_documents
    data = [
      ["apple", "green"],
      ["apple", "red"],
      ["orange", "orange"],
      ["grape", "green"],
      ["grape", "purple"],
      ["mandarin", "orange"],
      ["peach", "orange"],
      ["apricot", "orange"]
    ]

    docs = []

    data.each do |food|
      doc = Document::Document.new()
      doc << Field.new("name", food[0], Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
      doc << Field.new("colour", food[1], Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
      docs << doc
    end
    return docs
  end

  def IndexTestHelper.write_document(dir, doc, segment="test", analyzer = WhiteSpaceAnalyzer.new(), similarity = Similarity.default())
    writer = DocumentWriter.new(dir, analyzer, similarity, 50)
    writer.add_document(segment, doc)
  end

  def IndexTestHelper.prepare_book_list
    books = [
      {"author" => "P.H. Newby", "title" => "Something To Answer For", "year" => "1969"},
      {"author" => "Bernice Rubens", "title" => "The Elected Member", "year" => "1970"},
      {"author" => "V. S. Naipaul", "title" => "In a Free State", "year" => "1971"},
      {"author" => "John Berger", "title" => "G", "year" => "1972"},
      {"author" => "J. G. Farrell", "title" => "The Siege of Krishnapur", "year" => "1973"},
      {"author" => "Stanley Middleton", "title" => "Holiday", "year" => "1974"},
      {"author" => "Nadine Gordimer", "title" => "The Conservationist", "year" => "1974"},
      {"author" => "Ruth Prawer Jhabvala", "title" => "Heat and Dust", "year" => "1975"},
      {"author" => "David Storey", "title" => "Saville", "year" => "1976"},
      {"author" => "Paul Scott", "title" => "Staying On", "year" => "1977"},
      {"author" => "Iris Murdoch", "title" => "The Sea", "year" => "1978"},
      {"author" => "Penelope Fitzgerald", "title" => "Offshore", "year" => "1979"},
      {"author" => "William Golding", "title" => "Rites of Passage", "year" => "1980"},
      {"author" => "Salman Rushdie", "title" => "Midnight's Children", "year" => "1981"},
      {"author" => "Thomas Keneally", "title" => "Schindler's Ark", "year" => "1982"},
      {"author" => "J. M. Coetzee", "title" => "Life and Times of Michael K", "year" => "1983"},
      {"author" => "Anita Brookner", "title" => "Hotel du Lac", "year" => "1984"},
      {"author" => "Keri Hulme", "title" => "The Bone People", "year" => "1985"},
      {"author" => "Kingsley Amis", "title" => "The Old Devils", "year" => "1986"},
      {"author" => "Penelope Lively", "title" => "Moon Tiger", "year" => "1987"},
      {"author" => "Peter Carey", "title" => "Oscar and Lucinda", "year" => "1988"},
      {"author" => "Kazuo Ishiguro", "title" => "The Remains of the Day", "year" => "1989"},
      {"author" => "A. S. Byatt", "title" => "Possession", "year" => "1990"},
      {"author" => "Ben Okri", "title" => "The Famished Road", "year" => "1991"},
      {"author" => "Michael Ondaatje", "title" => "The English Patient", "year" => "1992"},
      {"author" => "Barry Unsworth", "title" => "Sacred Hunger", "year" => "1992"},
      {"author" => "Roddy Doyle", "title" => "Paddy Clarke Ha Ha Ha", "year" => "1993"},
      {"author" => "James Kelman", "title" => "How Late It Was, How Late", "year" => "1994"},
      {"author" => "Pat Barker", "title" => "The Ghost Road", "year" => "1995"},
      {"author" => "Graham Swift", "title" => "Last Orders", "year" => "1996"},
      {"author" => "Arundati Roy", "title" => "The God of Small Things", "year" => "1997"},
      {"author" => "Ian McEwan", "title" => "Amsterdam", "year" => "1998"},
      {"author" => "J. M. Coetzee", "title" => "Disgrace", "year" => "1999"},
      {"author" => "Margaret Atwood", "title" => "The Blind Assassin", "year" => "2000"},
      {"author" => "Peter Carey", "title" => "True History of the Kelly Gang", "year" => "2001"},
      {"author" => "Yann Martel", "title" => "The Life of Pi", "year" => "2002"},
      {"author" => "DBC Pierre", "title" => "Vernon God Little", "year" => "2003"}
    ]
    docs = []

    books.each do |book|
      doc = Document::Document.new()
      doc << Field.new("author", book["author"], Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
      doc << Field.new("title", book["title"], Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
      doc << Field.new("year", book["year"], Field::Store::YES, Field::Index::NO, Field::TermVector::NO)
      docs << doc
    end
    return docs
  end
end

