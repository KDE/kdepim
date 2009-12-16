#!/usr/bin/perl -w

use strict;

# This script updates some configuration keys

# read the whole config file
my $currentGroup = "";
my %configFile;
while ( <> ) {
  chomp; # eat the trailing '\n'
  next if ( /^$/ ); # skip empty lines
  next if ( /^\#/ ); # skip comments
  if ( /^\[/ ) { # group begin
    $currentGroup = $_;
    next;
  } elsif ( $currentGroup =~ /^\[Filter #[0-9]+\]$/ && /^Icon=/ ) {
    my ($key,$value) = split /=/;
    print "# DELETE $currentGroup$key\n${currentGroup}\nIcon=mail_spam\n" if $value eq "mark_as_spam";
    print "# DELETE $currentGroup$key\n${currentGroup}\nIcon=mail_ham\n" if $value eq "mark_as_ham";
  }
}
