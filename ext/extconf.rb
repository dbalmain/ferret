# extconf.rb for Ferret extensions
if (/mswin/ =~ RUBY_PLATFORM) and ENV['make'].nil?
  File.open('Makefile', "w") {}
  begin
    `nmake`
    require 'mkmf'
    create_makefile("ferret_ext")
  rescue => error
    require 'fileutils'
    FileUtils.copy('dummy.exe', 'nmake.exe')
  end
else
  require 'mkmf'
  $CFLAGS += " -fno-common"
  create_makefile("ferret_ext")
end
