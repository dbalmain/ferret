require 'ferret'

NUM_WORDS = 5

def hash_doc(filename)
  stk = Ferret::Analysis::StandardTokenizer.new("")
  words = []
  hashes = []
  File.open(filename) do |f|
    f.each do |line|
      stk.text = line
      while tk = stk.next()
        words << tk.text
        if words.size == NUM_WORDS
          hashes << words.hash
          words.shift
        end
      end
    end
  end
  return hashes.sort!
end

def hash_cmp(hash1, hash2)
  same = 0
  size_avg = (hash1.size + hash2.size)/2
  h1 = hash1.pop
  h2 = hash2.pop
  while (not hash1.empty? and not hash2.empty?)
    if (h2 == h1) 
      same += 1
      h1 = hash1.pop
      h2 = hash2.pop
    else
      if (h1 > h2)
        h1 = hash1.pop
      else
        h2 = hash2.pop
      end
    end
  end
  return same.to_f/size_avg
end

puts hash_cmp(hash_doc(ARGV[0]), hash_doc(ARGV[1]))
