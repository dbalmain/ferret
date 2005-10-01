$:.unshift File.join(File.dirname(__FILE__), '../../lib')

require 'ferret'

include Ferret::Store


vints = [ 9223372036854775807,
          0x00,
          0xFFFFFFFFFFFFFFFF]
t = Time.new
10.times do
  dpath = File.join(File.dirname(__FILE__),
                       'fsdir')
  dir = FSDirectory.get_directory(dpath, true)

  100.times do
    ostream = dir.create_output("rw_vint.test")
    300.times { |i| ostream.write_vint(vints[i%3]) }
    ostream.close
    istream = dir.open_input("rw_vint.test")
    300.times { istream.read_vint }
    istream.close
  end

  dir.close
end

puts "took #{Time.new - t} seconds"
