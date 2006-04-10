require File.dirname(__FILE__) + "/../../test_helper"


class SpansBasicTest < Test::Unit::TestCase
  include Ferret::Document
  include Ferret::Search
  include Ferret::Store
  include Ferret::Index
  include Ferret::Search::Spans
  include Ferret::Analysis

  def setup()
    data = [
      "start finish one two three four five six seven",
      "start one finish two three four five six seven",
      "start one two finish three four five six seven",
      "start one two three finish four five six seven",
      "start one two three four finish five six seven",
      "start one two three four five finish six seven",
      "start one two three four five six finish seven eight",
      "start one two three four five six seven finish eight nine",
      "start one two three four five six finish seven eight",
      "start one two three four five finish six seven",
      "start one two three four finish five six seven",
      "start one two three finish four five six seven",
      "start one two finish three four five six seven",
      "start one finish two three four five six seven",
      "start finish one two three four five six seven",
      "start start  one two three four five six seven",
      "finish start one two three four five six seven",
      "finish one start two three four five six seven",
      "finish one two start three four five six seven",
      "finish one two three start four five six seven",
      "finish one two three four start five six seven",
      "finish one two three four five start six seven",
      "finish one two three four five six start seven eight",
      "finish one two three four five six seven start eight nine",
      "finish one two three four five six start seven eight",
      "finish one two three four five start six seven",
      "finish one two three four start five six seven",
      "finish one two three start four five six seven",
      "finish one two start three four five six seven",
      "finish one start two three four five six seven",
      "finish start one two three four five six seven"
    ]
    @dir = RAMDirectory.new
    iw = IndexWriter.new(@dir, :analyzer => WhiteSpaceAnalyzer.new(), :create => true)
    data.each do |line|
      doc = Document.new()
      doc << Field.new("field", line, Field::Store::NO, Field::Index::TOKENIZED)
      iw << doc
    end

    iw.close()

    @is = IndexSearcher.new(@dir)
  end

  def tear_down()
    @iw.close
    @dir.close
  end

  def number_split(i)
    if (i < 10)
      return "<#{i}>"
    elsif (i < 100)
      return "<#{((i/10)*10)}> <#{i%10}>"
    else
      return "<#{((i/100)*100)}> <#{(((i%100)/10)*10)}> <#{i%10}>"
    end
  end

  def check_hits(query, expected, test_explain = false, top=nil)
    top_docs = @is.search(query, {:num_docs => expected.length})
    assert_equal(expected.length, top_docs.score_docs.size)
    assert_equal(top, top_docs.score_docs[0].doc) if top
    assert_equal(expected.length, top_docs.total_hits)
    top_docs.score_docs.each do |score_doc|
      assert(expected.include?(score_doc.doc),
             "#{score_doc.doc} was found unexpectedly")
      if test_explain
        assert(score_doc.score =~ @is.explain(query, score_doc.doc).value, 
          "Scores(#{score_doc.score} != #{@is.explain(query, score_doc.doc).value})")
      end
    end
  end

  def test_span_term_query()
    tq = SpanTermQuery.new(Term.new("field", "nine"))
    check_hits(tq, [7,23], true)
    tq = SpanTermQuery.new(Term.new("field", "eight"))
    check_hits(tq, [6,7,8,22,23,24])
  end

  def test_span_near_query()
    tq1 = SpanTermQuery.new(Term.new("field", "start"))
    tq2 = SpanTermQuery.new(Term.new("field", "finish"))
    q = SpanNearQuery.new([tq1, tq2], 0, true)
    check_hits(q, [0,14], true)
    q = SpanNearQuery.new([tq1, tq2], 0, false)
    check_hits(q, [0,14,16,30], true)
    q = SpanNearQuery.new([tq1, tq2], 1, true)
    check_hits(q, [0,1,13,14])
    q = SpanNearQuery.new([tq1, tq2], 1, false)
    check_hits(q, [0,1,13,14,16,17,29,30])
    q = SpanNearQuery.new([tq1, tq2], 4, true)
    check_hits(q, [0,1,2,3,4,10,11,12,13,14])
    q = SpanNearQuery.new([tq1, tq2], 4, false)
    check_hits(q, [0,1,2,3,4,10,11,12,13,14,16,17,18,19,20,26,27,28,29,30])
  end

  def test_span_not_query()
    tq1 = SpanTermQuery.new(Term.new("field", "start"))
    tq2 = SpanTermQuery.new(Term.new("field", "finish"))
    tq3 = SpanTermQuery.new(Term.new("field", "two"))
    tq4 = SpanTermQuery.new(Term.new("field", "five"))
    nearq1 = SpanNearQuery.new([tq1, tq2], 4, true)
    nearq2 = SpanNearQuery.new([tq3, tq4], 4, true)
    q = SpanNotQuery.new(nearq1, nearq2)
    check_hits(q, [0,1,13,14], true)
    nearq1 = SpanNearQuery.new([tq1, tq2], 4, false)
    q = SpanNotQuery.new(nearq1, nearq2)
    check_hits(q, [0,1,13,14,16,17,29,30])
    nearq1 = SpanNearQuery.new([tq1, tq3], 4, true)
    nearq2 = SpanNearQuery.new([tq2, tq4], 8, false)
    q = SpanNotQuery.new(nearq1, nearq2)
    check_hits(q, [2,3,4,5,6,7,8,9,10,11,12,15])
  end

  def test_span_first_query()
    finish_first = [16,17,18,19,20,21,22,23,24,25,26,27,28,29,30]
    tq = SpanTermQuery.new(Term.new("field", "finish"))
    q = SpanFirstQuery.new(tq, 1)
    check_hits(q, finish_first, true)
    q = SpanFirstQuery.new(tq, 5)
    check_hits(q, [0,1,2,3,11,12,13,14]+finish_first, false)
  end

  def test_span_or_query_query()
    tq1 = SpanTermQuery.new(Term.new("field", "start"))
    tq2 = SpanTermQuery.new(Term.new("field", "finish"))
    tq3 = SpanTermQuery.new(Term.new("field", "five"))
    nearq1 = SpanNearQuery.new([tq1, tq2], 1, true)
    nearq2 = SpanNearQuery.new([tq2, tq3], 0, false)
    q = SpanOrQuery.new([nearq1, nearq2])
    check_hits(q, [0,1,4,5,9,10,13,14], false)
    nearq1 = SpanNearQuery.new([tq1, tq2], 0, false)
    nearq2 = SpanNearQuery.new([tq2, tq3], 1, false)
    q = SpanOrQuery.new([nearq1, nearq2])
    check_hits(q, [0,3,4,5,6,8,9,10,11,14,16,30], false)
  end
end
