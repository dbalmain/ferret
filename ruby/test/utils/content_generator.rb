module ContentGenerator
  wpath = File.expand_path(File.join(__FILE__, '../../../data/words'))
  WORDS = File.readlines(wpath).collect {|w| w.strip}
  CHARS = 'abcdefghijklmnopqrstuvwxyz1234567890`~!@#$%^&*()_-+={[}]|\\:;"\'<,>.?/'
  ALNUM = 'abcdefghijklmnopqrstuvwxyz1234567890'
  ALPHA = 'abcdefghijklmnopqrstuvwxyz'
  URL_SUFFIXES = %w{com net org biz info}
  URL_COUNTRY_CODES = %w{au jp uk nz tv}
  TEXT_CACHE = {}
  WORD_CACHE = {}
  MARKDOWN_EMPHASIS_MARKERS = %w{* _ ** __ ` ``}
  MARKDOWN_LIST_MARKERS = %w{- * + 1.}

  def self.generate_text(length = 5..10, options = {})
    if length.is_a?(Range)
      raise ArgumentError, "range must be positive" unless length.min
      length = length.min + rand(length.max - length.min)
    end

    text = ''
    if options[:chars]
      while word = random_word and text.size + word.size < length
        text << word + ' '
      end
      text.strip!
      text << generate_word(length - text.size)
    else
      text = Array.new(length) {|x| random_word}.join(' ')
    end
    if key = options[:unique]||options[:key]
      cache = TEXT_CACHE[key]||={}
      if cache[text]
        return generate_text(options)
      else
        return cache[text] = true
      end      
    end
    return text
  end

  def self.generate_word(length = 5..10, options = {})
    if length.is_a?(Range)
      raise ArgumentError, "range must be positive" unless length.min
      length = length.min + rand(length.max - length.min)
    end

    word = ''
    case options[:charset]
    when :alpha
      word = Array.new(length) {|x| random_alpha}.pack('c*')
    when :alnum
      word = Array.new(length) {|x| random_alnum}.pack('c*')
    else
      word = Array.new(length) {|x| random_char}.pack('c*')
    end

    if key = options[:unique]||options[:key]
      cache = WORD_CACHE[key]||={}
      if cache[word]
        return generate_word(options)
      else
        cache[word] = true
      end      
    end
    return word
  end

  def self.generate_alpha_word(length = 5..10, options = {})
    options[:charset] = :alpha
    generate_word(length, options)
  end

  def self.generate_alnum_word(length = 5..10, options = {})
    options[:charset] = :alnum
    generate_word(length, options)
  end

  def self.generate_email(options = {})
    num_name_sections = 1 + rand(2)
    num_url_sections = 1 + rand(2)
    name = Array.new(num_name_sections) {|x| generate_alnum_word }.join('.')
    url = [generate_alnum_word]
    url += Array.new(num_url_sections) {|x| generate_alpha_word(2..3) }
    url = url.join('.')
    name + '@' + url
  end

  def self.generate_url(options = {})
    ext = random_from(URL_SUFFIXES)
    ext += '.' + random_from(URL_COUNTRY_CODES) if rand(2) > 0
    "http://www.#{generate_alnum_word}.#{ext}/"
  end

  def self.generate_markdown(length = 100..1000, options = {})
    @footnote_num = 0
    if length.is_a?(Range)
      raise ArgumentError, "range must be positive" unless length.min
      length = length.min + rand(length.max - length.min)
    end
    text = []
    while length > 0
      case rand
      when 0.3..1 # generate paragraph
        l = gen_num(length, 50)
        paragraph = gen_md_para(l)
        if rand > 0.95 # make block quote
          paragraph = '> ' + paragraph
        end
        text << paragraph
        length -= l
      when 0.2..0.3 # generate list
        li = random_from(MARKDOWN_LIST_MARKERS) + ' '
        num_elements = gen_num(length/5, 10)
        num_elements.times do
          break if length == 0
          if rand > 0.75 # do paragraph list element
            xli = li
            (2 + rand(3)).times do |i|
              break if length == 0
              l = gen_num(length, 10)
              text << xli
              text << gen_md_para(l, :no_footnotes => true)
              text << "\n\n"
              xli = ' ' * xli.size if i == 0
              length -= l
            end
          else
            l = gen_num(length, 10)
            text << li
            text << gen_md_para(l, :no_footnotes => true)
            text << "\n"
            length -= l
          end
        end
      when 0.1..0.2 # header
        l = gen_num(length, 7)
        t = gen_md_para(l, :no_footnotes => true)
        if rand > 0.8
          t += "\n" + random_from(%w{= -}) * t.size
        else
          t = ('#' * (1 + rand(6))) + ' ' + t
        end
        length -= l
        text << t
      else 
        text << '---'
      end
      text << "\n\n"
    end
    text.join()
  end

  def self.random_word
    random_from(WORDS)
  end

  def self.random_char
    random_from(CHARS)
  end

  def self.random_alnum
    random_from(ALNUM)
  end

  def self.random_alpha
    random_from(ALPHA)
  end

  private
  
  def self.gen_md_para(length, options = {})
    link_words = rand(1 + length/10)
    length -= link_words
    text = gen_md_text(length)
    text << "\n"
    footnote_cnt = 0
    while link_words > 0
      if options[:no_footnotes] or rand > 0.5
        if rand > 0.6 # inline link
          l = gen_num(link_words, 5)
          link = "[#{gen_md_text(l)}](#{generate_url} \"#{generate_text(1 + rand(5))}\")"
          text.insert(rand(text.length - footnote_cnt), link)
          link_words -= l
        else          # auto link
          text.insert(rand(text.length - footnote_cnt), "<#{generate_url}>")
          link_words -= 1
        end
      else            # footnote link
        l = gen_num(link_words, 5)
        reference = "[#{gen_md_text(l).join(' ')}][#{@footnote_num}]"
        text.insert(rand(text.length - footnote_cnt), reference)
        text << link = "\n[#{@footnote_num}]: #{generate_url} \"#{generate_text(1 + rand(5))}\""
        @footnote_num += 1
        footnote_cnt += 1
        link_words -= l
      end
    end
    text.pop if text.last == "\n"
    text.join(' ')
  end

  def self.gen_md_text(length)
    text = Array.new(length) {|x| random_word}
    if rand > 0.8
      (1 + rand(Math.sqrt(length))).times do
        first = rand(text.size)
        last = first + rand(3)
        last = text.size - 1 if last >= text.size
        words = text.slice!(first..last)
        em = random_from(MARKDOWN_EMPHASIS_MARKERS)
        words = "#{em}#{words.join(' ')}#{em}" unless words.join.index(em[0,1])
        text.insert(first, words).flatten!
      end
    end
    text
  end

  def self.gen_num(max1, max2)
    minmax = [max1, max2].min
    return minmax == 0 ? 0 : 1 + rand(minmax)
  end

  def self.random_from(list)
    list[rand(list.size)]
  end
end
