$:. << 'lib'
# Some parts of this Rakefile where taken from Jim Weirich's Rakefile for
# Rake. Other parts where stolen from the David Heinemeier Hansson's Rails
# Rakefile. Both are under MIT-LICENSE. Thanks to both for their excellent
# projects.

require 'rake'
require 'rake/testtask'
require 'rake/rdoctask'
require 'rake/clean'
require 'rake_utils/code_statistics'
require 'lib/ferret'

begin
  require 'rubygems'
  require 'rake/gempackagetask'
rescue Exception
  nil
end

CURRENT_VERSION = Ferret::VERSION
if ENV['REL']
  PKG_VERSION = ENV['REL']
else
  PKG_VERSION = CURRENT_VERSION
end

def announce(msg='')
  STDERR.puts msg
end

$VERBOSE = nil
CLEAN.include(FileList['**/*.o', 'InstalledFiles', '.config'])
CLOBBER.include(FileList['**/*.so'], 'ext/Makefile')

task :default => :all_tests
desc "Run all tests"
task :all_tests => [ :test_units, :test_functional ]

desc "Generate API documentation, and show coding stats"
task :doc => [ :stats, :appdoc ]

desc "run unit tests in test/unit"
Rake::TestTask.new("test_units" => :parsers) do |t|
  t.libs << "test/unit"
  t.pattern = 'test/unit/t[cs]_*.rb'
  t.verbose = true
end

desc "run unit tests in test/unit"
Rake::TestTask.new("test_long") do |t|
  t.libs << "test"
  t.libs << "test/unit"
  t.test_files = FileList["test/longrunning/tm_store.rb"]
  t.pattern = 'test/unit/t[cs]_*.rb'
  t.verbose = true
end

desc "run funtional tests in test/funtional"
Rake::TestTask.new("test_functional") do |t|
  t.libs << "test"
  t.pattern = 'test/funtional/tc_*.rb'
  t.verbose = true
end

desc "Report code statistics (KLOCS, etc) from application"
task :stats do
  CodeStatistics.new(
                      ["Ferret", "lib/ferret"],
                      ["Units", "test/unit"],
                      ["Units-extended", "test/longrunning"]
                    ).to_s
end

desc "Generate documentation for the application"
rd = Rake::RDocTask.new("appdoc") do |rdoc|
  rdoc.rdoc_dir = 'doc/api'
  rdoc.title    = "Ferret Search Library Documentation"
  rdoc.options << '--line-numbers --inline-source'
  rdoc.rdoc_files.include('README')
  rdoc.rdoc_files.include('TODO')
  rdoc.rdoc_files.include('TUTORIAL')
  rdoc.rdoc_files.include('MIT-LICENSE')
  rdoc.rdoc_files.include('lib/**/*.rb')
end

EXT = "ferret_ext.so"

desc "Build the extension"
task :ext => "ext/#{EXT}"

file "ext/#{EXT}" => "ext/Makefile" do
  sh "cd ext; make"
end

file "ext/Makefile" do
  sh "cd ext; ruby extconf.rb"
end

# Make Parsers ---------------------------------------------------------------

RACC_SRC = FileList["**/*.y"]
RACC_OUT = RACC_SRC.collect { |fn| fn.sub(/\.y$/, '.tab.rb') }

task :parsers => RACC_OUT
rule(/\.tab\.rb$/ => [proc {|tn| tn.sub(/\.tab\.rb$/, '.y')}]) do |t|
  sh "racc #{t.source}" 
end

# Create Packages ------------------------------------------------------------

task :release => [:package]
PKG_DIR = "../packages"

directory PKG_DIR

PKG_FILES = FileList[
  'setup.rb',
  '[-A-Z]*',
  'ext/**/*', 
  'lib/**/*.rb', 
  'test/**/*.rb',
  'rake_utils/**/*.rb',
  'Rakefile'
]
PKG_FILES.exclude('**/*.o')


if ! defined?(Gem)
  puts "Package Target requires RubyGEMs"
else
  spec = Gem::Specification.new do |s|
    
    #### Basic information.

    s.name = 'ferret'
    s.version = PKG_VERSION
    s.summary = "Ruby indexing library."
    s.description = <<-EOF
      Ferret is a port of the Java Lucene project. It is a powerful
      indexing and search library.
    EOF

    #### Dependencies and requirements.

    #s.add_dependency('log4r', '> 1.0.4')
    #s.requirements << ""

    #### Which files are to be included in this gem?  Everything!  (Except CVS directories.)

    s.files = PKG_FILES.to_a

    #### C code extensions.

    s.extensions << "ext/extconf.rb"

    #### Load-time details: library and application (you will need one or both).

    s.require_path = 'lib'                         # Use these for libraries.

    #s.bindir = "bin"                               # Use these for applications.
    #s.executables = ["rake"]
    #s.default_executable = "rake"

    #### Documentation and testing.

    s.has_rdoc = true
    s.extra_rdoc_files = rd.rdoc_files.reject { |fn| fn =~ /\.rb$/ }.to_a
    s.rdoc_options <<
      '--title' <<  'Ferret -- Ruby Indexer' <<
      '--main' << 'README' << '--line-numbers' <<
      'TUTORIAL' << 'TODO'

    #### Author and project details.

    s.author = "David Balmain"
    s.email = "dbalmain@gmail.com"
    s.homepage = "http://ferret.davebalmain.com"
    s.rubyforge_project = "ferret"
#     if ENV['CERT_DIR']
#       s.signing_key = File.join(ENV['CERT_DIR'], 'gem-private_key.pem')
#       s.cert_chain  = [File.join(ENV['CERT_DIR'], 'gem-public_cert.pem')]
#     end
  end

  package_task = Rake::GemPackageTask.new(spec) do |pkg|
    pkg.need_zip = true
    pkg.need_tar = true
  end
end

# Support Tasks ------------------------------------------------------

desc "Look for TODO and FIXME tags in the code"
task :todo do
  FileList['**/*.rb'].egrep /#.*(FIXME|TODO|TBD)/
end
# --------------------------------------------------------------------
# Creating a release

desc "Make a new release"
task :release => [
  :prerelease,
  :clobber,
  :all_tests,
  :package,
  :tag] do
  
  announce 
  announce "**************************************************************"
  announce "* Release #{PKG_VERSION} Complete."
  announce "* Packages ready to upload."
  announce "**************************************************************"
  announce 
end

# Validate that everything is ready to go for a release.
task :prerelease do
  announce 
  announce "**************************************************************"
  announce "* Making RubyGem Release #{PKG_VERSION}"
  announce "* (current version #{CURRENT_VERSION})"
  announce "**************************************************************"
  announce  

  # Is a release number supplied?
  unless ENV['REL']
    fail "Usage: rake release REL=x.y.z [REUSE=tag_suffix]"
  end

  # Is the release different than the current release.
  # (or is REUSE set?)
  if PKG_VERSION == CURRENT_VERSION && ! ENV['REUSE']
    fail "Current version is #{PKG_VERSION}, must specify REUSE=tag_suffix to reuse version"
  end

  # Are all source files checked in?
  data = `svn -q status`
  unless data =~ /^$/
    fail "'svn -q status' is not clean ... do you have unchecked-in files?"
  end
  announce "No outstanding checkins found ... OK"
end

task :update_version => [:prerelease] do
  if PKG_VERSION == CURRENT_VERSION
    announce "No version change ... skipping version update"
  else
    announce "Updating Ferret version to #{PKG_VERSION}"
    open("lib/ferret.rb") do |ferret_in|
      open("lib/ferret.rb.new", "w") do |ferret_out|
        ferret_in.each do |line|
          if line =~ /^  VERSION\s*=\s*/
            ferret_out.puts "  VERSION = '#{PKG_VERSION}'"
          else
            ferret_out.puts line
          end
        end
      end
    end
    if ENV['RELTEST']
      announce "Release Task Testing, skipping commiting of new version"
    else
      mv "lib/ferret.rb.new", "lib/ferret.rb"
    end
    sh %{svn ci -m "Updated to version #{PKG_VERSION}" lib/ferret.rb}
  end
end

desc "Tag all the SVN files with the latest release number (REL=x.y.z)"
task :tag => [:prerelease] do
  reltag = "REL-#{PKG_VERSION}"
  reltag << ENV['REUSE'] if ENV['REUSE']
  announce "Tagging SVN with [#{reltag}]"
  if ENV['RELTEST']
    announce "Release Task Testing, skipping CVS tagging. Would do the following;"
    announce %{svn copy -m "creating release #{reltag}" svn://www.davebalmain.com/ferret/trunk svn://www.davebalmain.com/ferret/tags/#{reltag}}
  else
    sh %{svn copy -m "creating release #{reltag}" svn://www.davebalmain.com/ferret/trunk svn://www.davebalmain.com/ferret/tags/#{reltag}}
  end
end
