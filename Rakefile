require 'rake'
require 'rake/testtask'
require 'rake/rdoctask'

$VERBOSE = nil

require 'code_statistics'

desc "Run all tests"
task :default => [ :test_units, :test_functional ]

desc "Generate API documentation, and show coding stats"
task :doc => [ :stats, :appdoc ]

desc "run unit tests in test/unit"
Rake::TestTask.new("test_units") { |t|
  t.libs << "test/unit"
  t.pattern = 'test/unit/t[cs]_*.rb'
  t.verbose = true
}

desc "run unit tests in test/unit"
Rake::TestTask.new("test_units_longrunning") { |t|
  t.libs << "test"
  t.libs << "test/unit"
  t.test_files = FileList["test/longrunning/tm_store.rb"]
  t.pattern = 'test/unit/t[cs]_*.rb'
  t.verbose = true
}

desc "run funtional tests in test/funtional"
Rake::TestTask.new("test_functional") { |t|
  t.libs << "test"
  t.pattern = 'test/funtional/tc_*.rb'
  t.verbose = true
}

desc "Report code statistics (KLOCS, etc) from application"
task :stats do
  CodeStatistics.new(
                      ["Store", "lib/store"],
                      ["Index", "lib/index"],
                      ["Analysis", "lib/analysis"],
                      ["Utils", "lib/utils"],
                      ["Search", "lib/search"],
                      ["Document", "lib/document"],
                      ["Units", "test/unit"],
                      ["Units-extended", "test/longrunning"]
                    ).to_s
end

desc "Generate documentation for the application"
Rake::RDocTask.new("appdoc") do |rdoc|
  rdoc.rdoc_dir = 'doc/api'
  rdoc.title    = "Ferret Search Library Documentation"
  rdoc.options << '--line-numbers --inline-source'
  rdoc.rdoc_files.include('doc/README')
  rdoc.rdoc_files.include('lib/**/*.rb')
end
