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
  } elsif ( $currentGroup /^[Resource_.*]/ and /^ResourceType/ ) {
    my ($key,$value) = split /=/;
    if ( $value eq "kolab" ) {
      print "# DELETE $currentGroup$key\n";
      print "[$currentGroup]\nResourceType=imap\n";
    }
  }
}
