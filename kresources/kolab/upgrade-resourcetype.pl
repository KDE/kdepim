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
  if ( /^\[(.+)\]$/ ) { # group begin
    $currentGroup = $1;
    next;
  } elsif ( $currentGroup =~ /^Resource/ ) {
    my ($key,$value) = split /=/;
    if ( $key eq "ResourceType" and $value eq "kolab" ) {
      print "# DELETE [$currentGroup]$key\n";
      print "[$currentGroup]\nResourceType=imap\n";
    }
  }
}
