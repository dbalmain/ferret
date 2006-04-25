    require 'ferret'
    $stdin.each do |line|
      stk = Ferret::Analysis::StandardTokenizer.new(line)
      while tk = stk.next()
        puts "    <#{tk.text}> from #{tk.start_offset} to #{tk.end_offset}"
      end
    end
