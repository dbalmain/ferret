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

  IR_TEST_DOC_CNT = 64

  def IndexTestHelper.prepare_ir_test_docs()
    body = "body"
    title = "title"
    author = "author"
    text = "text"
    year = "year"
    changing_field = "changing_field"

    docs = Array.new(IR_TEST_DOC_CNT)
    docs[0] = Document.new()
    docs[0] << Field.new(body, "Where is Wally", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    docs[0] << Field.new(changing_field, "word3 word4 word1 word2 word1 word3 word4 word1 word3 word3", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::NO)
    docs[1] = Document.new()
    docs[1] << Field.new(body, "Some Random Sentence read", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    docs[2] = Document.new()
    docs[2] << Field.new(body, "Some read Random Sentence read", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    docs[3] = Document.new()
    docs[3] << Field.new(title, "War And Peace", Field::Store::YES, Field::Index::UNTOKENIZED, Field::TermVector::WITH_OFFSETS)
    docs[3] << Field.new(body, "word3 word4 word1 word2 word1 word3 word4 word1 word3 word3", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    docs[3] << Field.new(author, "Leo Tolstoy", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS)
    docs[3] << Field.new(year, "1865", Field::Store::YES, Field::Index::NO, Field::TermVector::NO)
    docs[3] << Field.new(text, "more text which is not stored", Field::Store::NO, Field::Index::TOKENIZED, Field::TermVector::NO)
    docs[4] = Document.new()
    docs[4] << Field.new(body, "Some Random Sentence", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    docs[5] = Document.new()
    docs[5] << Field.new(body, "Here's Wally", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    docs[6] = Document.new()
    docs[6] << Field.new(body, "Some Random Sentence read read read read", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    docs[7] = Document.new()
    docs[7] << Field.new(body, "Some Random Sentence", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    docs[8] = Document.new()
    docs[8] << Field.new(body, "Some Random Sentence", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    docs[9] = Document.new()
    docs[9] << Field.new(body, "read Some Random Sentence read this will be used after unfinished next position read", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    docs[10] = Document.new()
    docs[10] << Field.new(body, "Some read Random Sentence", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    docs[10] << Field.new(changing_field, "word3 word4 word1 word2 word1 word3 word4 word1 word3 word3", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::YES)
    docs[11] = Document.new()
    docs[11] << Field.new(body, "And here too. Well, maybe Not", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    docs[12] = Document.new()
    docs[12] << Field.new(body, "Some Random Sentence", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    docs[13] = Document.new()
    docs[13] << Field.new(body, "Some Random Sentence", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    docs[14] = Document.new()
    docs[14] << Field.new(body, "Some Random Sentence", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    docs[15] = Document.new()
    docs[15] << Field.new(body, "Some read Random Sentence", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    docs[16] = Document.new()
    docs[16] << Field.new(body, "Some Random read read Sentence", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    docs[17] = Document.new()
    docs[17] << Field.new(body, "Some Random read Sentence", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    docs[17] << Field.new(changing_field, "word3 word4 word1 word2 word1 word3 word4 word1 word3 word3", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS)
    docs[18] = Document.new()
    docs[18] << Field.new(body, "Wally Wally Wally", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    docs[19] = Document.new()
    docs[19] << Field.new(body, "Some Random Sentence", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    docs[19] << Field.new(changing_field, "word3 word4 word1 word2 word1 word3 word4 word1 word3 word3", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_OFFSETS)
    docs[20] = Document.new()
    docs[20] << Field.new(body, "Wally is where Wally usually likes to go. Wally Mart! Wally likes shopping there for Where's Wally books. Wally likes to read", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    docs[20] << Field.new(changing_field, "word3 word4 word1 word2 word1 word3 word4 word1 word3 word3", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    docs[21] = Document.new()
    docs[21] << Field.new(body, "Some Random Sentence read read read and more read read read", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    docs[21] << Field.new(changing_field, "word3 word4 word1 word2 word1 word3 word4 word1 word3 word3", Field::Store::YES, Field::Index::TOKENIZED, Field::TermVector::NO)

    buf = ""
    21.times { buf << "skip " }
    22.upto(IR_TEST_DOC_CNT) do |i|
      buf << "skip "
      docs[i] = Document.new()
      docs[i] << Field.new(text, buf.clone, Field::Store::NO, Field::Index::TOKENIZED, Field::TermVector::WITH_POSITIONS_OFFSETS)
    end
    return docs
  end

  def IndexTestHelper.prepare_search_docs
    data = [
      {"field" => "word1"},
      {"field" => "word1 word2"},
      {"field" => "word1 word3"},
      {"field" => "word1 word3"},
      {"field" => "word1 word2"},
      {"field" => "word1"},
      {"field" => "word1 word3"},
      {"field" => "word1"},
      {"field" => "word1 word2 word3"},
      {"field" => "word1"},
      {"field" => "word1"},
      {"field" => "word1 word3"},
      {"field" => "word1"},
      {"field" => "word1"},
      {"field" => "word1 word3"},
      {"field" => "word1"},
      {"field" => "word1"},
      {"field" => "word1"}
    ]

    docs = []
    data.each_with_index do |fields, i|
      doc = Document::Document.new()
      doc.boost = i+1

      fields.each_pair do |field, text|
        doc << Field.new(field, text, Field::Store::NO, Field::Index::TOKENIZED, Field::TermVector::NO, i+1)
      end
      docs << doc
    end
    return docs
  end
end

