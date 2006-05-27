#!/usr/env perl
use strict;
use warnings;
use File::Find;

$| = 1;				# so we can see it run

my %dep_tree;
my %objs;

sub get_dep_tree
{
  -f and /\.[ch]$/ or return;
  my $file = $_;

  open(ARTICLE, $_) or return;

  while (my $line = (<ARTICLE>))
  {
    if ($line =~ /#include "(.*)"/) {
      #print($file.':'.$1."\n");
      $dep_tree{$file} = [] unless exists $dep_tree{$file};
      push(@{$dep_tree{$file}}, $1);
    }
  }
}

sub get_deps
{
  my ($file, $deps) = @_;

  foreach (@{$dep_tree{$file}}) {
    next if exists ${$deps}{$_};
    ${$deps}{$_} = 1;
    get_deps($_, $deps);
  }
}

sub get_objs
{
  -f and /\.c$/ or return;
  $objs{$_} = 1;
}

find(\&get_objs, "test");

print "CFLAGS = -ansi -pedantic -Wall -Iinclude -fno-common -O2 -g -DDEBUG

LFLAGS = -lm

TEST_OBJS =";

foreach (keys %objs) {
  s/\.c$/.o/;
  print " $_";
}
print "

OBJS = ";

%objs = ();

find(\&get_objs, "src");

foreach (keys %objs) {
  s/\.c$/.o/;
  print " $_";
}

print '

vpath %.c test src

vpath %.h test include

runtests: testall
	./testall -v -f -q

testall: $(OBJS) $(TEST_OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) $(OBJS) $(TEST_OBJS) -o testall

valgrind: testall
	valgrind --leak-check=yes -v testall -q

.PHONY: clean
clean:
	rm -f *.o testall gmon.out

';

find(\&get_dep_tree, ".");

foreach (keys %dep_tree) {
  next unless /\.c$/;
  my $cfile = $_;
  s/\.c$/.o/;
  print $_.':';
  my $deps = {};
  get_deps($cfile, $deps);
  foreach (keys %{$deps}) {
    print ' '.$_;
  }
  print "\n\n";
}

