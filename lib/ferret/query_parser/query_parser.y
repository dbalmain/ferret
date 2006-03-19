class Ferret::QueryParser

  prechigh
    left ':'
    nonassoc REQ NOT
    left AND OR
    nonassoc HIGH
    nonassoc LOW
  preclow

  options no_result_var

  expect 1
rule

  top_query     : bool_query
                  {
                    get_boolean_query(val[0])
                  }
                ;
  bool_query    : bool_clause
                  {
                    [val[0]]
                  }
                | bool_query AND bool_clause
                  {
                    add_and_clause(val[0], val[2])
                  }
                | bool_query OR bool_clause
                  {
                    add_or_clause(val[0], val[2])
                  }
                | bool_query bool_clause
                  {
                    add_default_clause(val[0], val[1])
                  }
                ;
  bool_clause   : REQ boosted_query
                  {
                    get_boolean_clause(val[1], BooleanClause::Occur::MUST)
                  }
                | NOT boosted_query
                  {
                    get_boolean_clause(val[1], BooleanClause::Occur::MUST_NOT)
                  }
                | boosted_query
                  {
                    get_boolean_clause(val[0], BooleanClause::Occur::SHOULD)
                  }
                ;
  boosted_query : query
                | query '^' WORD { val[0].boost = val[2].to_f; return val[0] }
                ;
  query         : term_query
                | '(' bool_query ')'
                  {
                    get_boolean_query(val[1])
                  }
                | field_query
                | phrase_query
                | range_query
                | wild_query
                ;
  term_query    : WORD
                  {
                    _get_term_query(val[0])
                  }
                | WORD '~' WORD =HIGH
                  {
                    _get_fuzzy_query(val[0], val[2])
                  }
                | WORD '~' =LOW
                  {
                    _get_fuzzy_query(val[0])
                  }
                ;
  wild_query    : WILD_STRING
                  {
                    _get_wild_query(val[0])
                  }
                ;
  field_query   : field ':' query {@field = @default_field}
                  {
                    val[2]
                  }
                | '*' {@field = "*"} ':' query {@field = @default_field}
                  {
                    val[3]
                  }
                ;
  field         : WORD               { @field = [val[0]] }
                | field '|' WORD     { @field = val[0] += [val[2]] }
                ;
  phrase_query  : '\"' phrase_words '\"'
                  {
                    get_phrase_query(val[1])
                  }
                | '\"' phrase_words '\"' '~' WORD
                  {
                    get_phrase_query(val[1], val[4].to_i)
                  }
                | '\"' '\"'                { nil }
                | '\"' '\"' '~' WORD       { nil }
                ;
  phrase_words  : WORD                     { [val[0]] }
                | phrase_words WORD        { val[0] << val[1]  }
                | phrase_words '<' '>'     { val[0] << nil  }
                | phrase_words '|' WORD    { add_multi_word(val[0], val[2])  }
                ;
  range_query   : '[' WORD WORD ']' { _get_range_query(val[1], val[2], true, true) }
                | '[' WORD WORD '}' { _get_range_query(val[1], val[2], true, false) }
                | '{' WORD WORD ']' { _get_range_query(val[1], val[2], false, true) }
                | '{' WORD WORD '}' { _get_range_query(val[1], val[2], false, false) }
                | '<' WORD '}'      { _get_range_query(nil,    val[1], false, false) }
                | '<' WORD ']'      { _get_range_query(nil,    val[1], false, true) }
                | '[' WORD '>'      { _get_range_query(val[1], nil,    true, false) }
                | '{' WORD '>'      { _get_range_query(val[1], nil,    false, false) }
                | '<' WORD          { _get_range_query(nil,    val[1], false, false) }
                | '<' '=' WORD      { _get_range_query(nil,    val[2], false, true) }
                | '>' '='  WORD     { _get_range_query(val[2], nil,    true, false) }
                | '>' WORD          { _get_range_query(val[1], nil,    false, false) }
                ;
end

---- inner
  attr_accessor :default_field, :fields, :handle_parse_errors

  def initialize(default_field = "*", options = {})
    @yydebug = true
    if default_field.is_a?(String) and default_field.index("|")
      default_field = default_field.split("|")
    end
    @field = @default_field = default_field
    @analyzer = options[:analyzer] || Analysis::Analyzer.new
    @wild_lower = options[:wild_lower].nil? ? true : options[:wild_lower]
    @occur_default = options[:occur_default] || BooleanClause::Occur::SHOULD
    @default_slop = options[:default_slop] || 0
    @fields = options[:fields]||[]
    @handle_parse_errors = options[:handle_parse_errors] || false
  end

  RESERVED = {
    'AND'    => :AND,
    '&&'     => :AND,
    'OR'     => :OR,
    '||'     => :OR,
    'NOT'    => :NOT,
    '!'      => :NOT,
    '-'      => :NOT,
    'REQ'    => :REQ,
    '+'      => :REQ
  }

  ECHR =  %q,:()\[\]{}!+"~^\-\|<>\=\*\?,
  EWCHR = %q,:()\[\]{}!+"~^\-\|<>\=,

  def parse(str)
    orig_str = str
    str = clean_string(str)
    str.strip!
    @q = []

    until str.empty? do
      case str
      when /\A\s+/
        ;
      when /\A([#{EWCHR}]|[*?](?=:))/
        @q.push [ RESERVED[$&]||$&, $& ]
      when /\A(\&\&|\|\|)/
        @q.push [ RESERVED[$&], $& ]
      when /\A(\\[#{ECHR}]|[^\s#{ECHR}])*[?*](\\[#{EWCHR}]|[^\s#{EWCHR}])*/
        str = $'
        unescaped = $&.gsub(/\\(?!\\)/,"")
        @q.push [ :WILD_STRING, unescaped ]
        next
      when /\A(\\[#{ECHR}]|[^\s#{ECHR}])+/
        symbol = RESERVED[$&]
        if symbol
          @q.push [ symbol, $& ]
        else
          str = $'
          unescaped = $&.gsub(/\\(?!\\)/,"")
          @q.push [ :WORD, unescaped ]
          next
        end
      else
        raise RuntimeError, "shouldn't happen"
      end
      str = $'
    end
    if @q.empty?
      return TermQuery.new(Term.new(@default_field, ""))
    end

    @q.push([ false, '$' ])

    query = nil
    begin
      query = do_parse
    rescue Racc::ParseError => e
      if @handle_parse_errors
        @field = @default_field
        query = _get_bad_query(orig_str)
      else
        raise QueryParseException.new("Could not parse #{str}", e)
      end
    end
    return query
  end

  def next_token
    @q.shift
  end

  PHRASE_CHARS = [?<, ?>, ?|, ?"] # these chars have meaning within phrases
  def clean_string(str)
    escape_chars = ECHR.gsub(/\\/,"").unpack("c*")
    pb = nil
    br_stack = []
    quote_open = false
    # leave a little extra
    new_str = []

    str.each_byte do |b|
      # ignore escaped characters
      if pb == ?\\
        if quote_open and PHRASE_CHARS.index(b)
          new_str << ?\\ # this was left off the first time through
        end

        new_str << b
        pb = (b == ?\\ ? ?: : b) # \\ has escaped itself so does nothing more
        next
      end
      case b
      when ?\\
        new_str << b if !quote_open # We do our own escaping below
      when ?"
        quote_open = !quote_open
        new_str << b
      when ?(
        if !quote_open
          br_stack << b
        else
          new_str << ?\\
        end
        new_str << b
      when ?)
        if !quote_open
          if br_stack.size == 0
            new_str.unshift(?()
          else
            br_stack.pop
          end
        else
          new_str << ?\\
        end
        new_str << b
      when ?>
        if quote_open
          if pb == ?<
            new_str.delete_at(-2)
          else
            new_str << ?\\
          end
        end
        new_str << b
      else
        if quote_open
          if escape_chars.index(b) and b != ?|
            new_str << ?\\
          end
        end
        new_str << b
      end
      pb = b
    end
    new_str << ?" if quote_open
    br_stack.each { |b| new_str << ?) }
    return new_str.pack("c*")  
  end

  def get_bad_query(field, str)
    get_term_query(field, str)
    #tokens = []
    #stream = @analyzer.token_stream(field, str)
    #while token = stream.next
    #  tokens << token
    #end
    #if tokens.length == 0
    #  return TermQuery.new(Term.new(field, ""))
    #elsif tokens.length == 1
    #  return TermQuery.new(Term.new(field, tokens[0].text))
    #else
    #  bq = BooleanQuery.new()
    #  tokens.each do |token|
    #    bq << BooleanClause.new(TermQuery.new(Term.new(field, token.text)))
    #  end
    #  return bq
    #end
  end

  def get_range_query(field, start_word, end_word, inc_upper, inc_lower)
     RangeQuery.new(field, start_word, end_word, inc_upper, inc_lower)
  end

  def get_term_query(field, word)
    tokens = []
    stream = @analyzer.token_stream(field, word)
    while token = stream.next
      tokens << token
    end
    if tokens.length == 0
      return TermQuery.new(Term.new(field, ""))
    elsif tokens.length == 1
      return TermQuery.new(Term.new(field, tokens[0].text))
    else
      pq = PhraseQuery.new()
      tokens.each do |token|
        pq.add(Term.new(field, token.text), nil, token.pos_inc)
      end
      return pq
    end
  end

  def get_fuzzy_query(field, word, min_sim = nil)
    tokens = []
    stream = @analyzer.token_stream(field, word)
    if token = stream.next # only makes sense to look at one term for fuzzy
      if min_sim
        return FuzzyQuery.new(Term.new(field, token.text), min_sim.to_f)
      else
        return FuzzyQuery.new(Term.new(field, token.text))
      end
    else
      return TermQuery.new(Term.new(field, ""))
    end
  end

  def get_wild_query(field, regexp)
    WildcardQuery.new(Term.new(field, regexp))
  end

  def add_multi_word(words, word)
    last_word = words[-1]
    if not last_word.is_a?(Array)
      last_word = words[-1] = [words[-1]]
    end
    last_word << word
    return words
  end

  def get_normal_phrase_query(field, positions)
    pq = PhraseQuery.new()
    pq.slop = @default_slop
    pos_inc = 0

    positions.each do |position|
      if position.nil?
        pos_inc += 1
        next
      end
      stream = @analyzer.token_stream(field, position)
      tokens = []
      while token = stream.next
        tokens << token
      end
      tokens.each do |token|
        pq.add(Term.new(field, token.text), nil,
               token.pos_inc + pos_inc)
        pos_inc = 0
      end
    end
    return pq
  end

  def get_multi_phrase_query(field, positions)
    mpq = MultiPhraseQuery.new()
    mpq.slop = @default_slop
    pos_inc = 0

    positions.each do |position|
      if position.nil?
        pos_inc += 1
        next
      end
      if position.is_a?(Array)
        position.compact! # it doesn't make sense to have an empty spot here
        terms = []
        position.each do |word|
          stream = @analyzer.token_stream(field, word)
          if token = stream.next # only put one term per word
            terms << Term.new(field, token.text)
          end
        end
        mpq.add(terms, nil, pos_inc + 1) # must go at least one forward
        pos_inc = 0
      else
        stream = @analyzer.token_stream(field, position)
        tokens = []
        while token = stream.next
          tokens << token
        end
        tokens.each do |token|
          mpq.add([Term.new(field, token.text)], nil,
                 token.pos_inc + pos_inc)
          pos_inc = 0
        end
      end
    end
    return mpq
  end

  def get_phrase_query(positions, slop = nil)
    if positions.size == 1 
      if positions[0].is_a?(Array)
        clauses = positions[0].map { |word|
          BooleanClause.new(_get_term_query(word), BooleanClause::Occur::SHOULD)
        }
        return get_boolean_query(clauses)
      else
        return _get_term_query(positions[0])
      end
    end

    multi_phrase = false
    positions.each do |position|
      if position.is_a?(Array)
        position.compact!
        if position.size > 1
          multi_phrase = true
        end
      end
    end

    return do_multiple_fields() do |field|
      q = nil
      if not multi_phrase
        q = get_normal_phrase_query(field, positions.flatten)
      else
        q = get_multi_phrase_query(field, positions)
      end
      q.slop = slop if slop
      next q
    end
  end

  def add_and_clause(clauses, clause)
    clauses.compact!
    if (clauses.length == 1)
      last_cl = clauses[0]
      last_cl.occur = BooleanClause::Occur::MUST if not last_cl.prohibited?
    end

    return if clause.nil? # incase a query got destroyed by the analyzer

    clause.occur = BooleanClause::Occur::MUST if not clause.prohibited?
    clauses << clause
  end

  def add_or_clause(clauses, clause)
    clauses << clause
  end

  def add_default_clause(clauses, clause)
    if @occur_default == BooleanClause::Occur::MUST
      add_and_clause(clauses, clause)
    else
      add_or_clause(clauses, clause)
    end
  end

  def get_boolean_query(clauses)
    # possible that we got all nil clauses so check
    return nil if clauses.nil?
    clauses.compact!
    return nil if clauses.size == 0

    if clauses.size == 1 and not clauses[0].prohibited?
      return clauses[0].query
    end
    bq = BooleanQuery.new()
    clauses.each {|clause| bq << clause }
    return bq                
  end                        
                             
  def get_boolean_clause(query, occur)
    return nil if query.nil?
    return BooleanClause.new(query, occur)
  end

  def do_multiple_fields()
    # set @field to all fields if @field is the multi-field operator
    @field = @fields if @field.is_a?(String) and @field == "*"
    if @field.is_a?(String)
      return yield(@field)
    elsif @field.size == 1
      return yield(@field[0])
    else
      bq = BooleanQuery.new()
      @field.each do |field|
        q = yield(field)
        bq << BooleanClause.new(q) if q
      end
      return bq                
    end
  end

  def method_missing(meth, *args)
    if meth.to_s =~ /_(get_[a-z_]+_query)/
      do_multiple_fields() do |field|
        send($1, *([field] + args))
      end
    else
      raise NoMethodError.new("No such method #{meth} in #{self.class}", meth, args)
    end
  end

  def QueryParser.parse(query, default_field = "*", options = {})
    qp = QueryParser.new(default_field, options)
    return qp.parse(query)
  end

---- footer

if __FILE__ == $0
  $:.unshift File.join(File.dirname(__FILE__), '..')
  $:.unshift File.join(File.dirname(__FILE__), '../..')
  require 'utils'
  require 'analysis'
  require 'document'
  require 'store'
  require 'index'
  require 'search'

  include Ferret::Search
  include Ferret::Index

  st = "\033[7m"
  en = "\033[m"

  parser = Ferret::QueryParser.new("default",
                                   :fields => ["f1", "f2", "f3"],
                                   :analyzer => Ferret::Analysis::StandardAnalyzer.new,
                                   :handle_parse_errors => true)

  $stdin.each do |line|
    query = parser.parse(line)
    if query
      puts "#{query.class}"
      puts query.to_s(parser.default_field)
    else
      puts "No query was returned"
    end
  end
end
