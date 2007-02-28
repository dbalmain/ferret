#!/usr/bin/env ruby
require 'net/http'
require 'rbconfig'
require 'uri'
require File.join(File.dirname(__FILE__), 'ruby/lib/ferret_version.rb')

EMAIL = ARGV[1]||'johnny.nobody@email.com'

Dir.chdir(File.join(File.dirname(__FILE__)))
IO.popen("svn update") do |io|
  svn_out = io.readlines
  puts svn_out.to_s
  exit unless svn_out.size > 1 or ARGV[0] =~ /force/i
end

ruby_v = nil
IO.popen('ruby -v') do |io|
  ruby_v = io.read
end

CC = Config::MAKEFILE_CONFIG['CC']
cc_v = nil
IO.popen("#{CC} -v 2>&1") do |io|
  cc_v = io.read
end

res = nil
Dir.chdir('c')
IO.popen('make') do |io|
  res = "<pre>#{io.read}</pre>"
end
c_test_results  = (res =~ /All tests passed./) ? 'Success' : res

Dir.chdir('../ruby')
`rake clobber` if ARGV[0] =~ /true/i
IO.popen('rake') do |io|
  res = "<pre>#{io.read}</pre>"
end
ruby_test_results  = (res =~ /0 failures, 0 errors/) ? 'Success' : res
IO.popen('ruby -v') do |io| 
  version_info = io.read.split(%r{[\s\(\)\[\]]+}) 
  @tested_ruby_version = version_info[1] 
  @tested_ruby_platform = version_info[3] 
  @tested_ruby_release = version_info[2] 
end 

results = {
  :ruby_v => ruby_v,
  :c_compiler => CC,
  :cc_v => cc_v,
  :ruby_version => @tested_ruby_version,
  :platform => @tested_ruby_platform,
  :release_date => @tested_ruby_release,
  :ferret_version => Ferret::VERSION,
  :c_test_results => c_test_results,
  :ruby_test_results => ruby_test_results,
  :email => EMAIL
}

res = Net::HTTP.post_form(URI.parse('http://camping.davebalmain.com/smoke_alarm/'), results)
