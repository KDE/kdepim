#!/usr/bin/perl

use strict;

my $inplace;

if ( @ARGV == 1 && $ARGV[ 0 ] == "--inplace" ) {
  print "Substituting in place.\n";
  $inplace = 1;
}

my $fileIn = "groupwise.h";
my $fileOut = "$fileIn.fixed";
my $fileBack = "$fileIn.backup";

if ( !open IN, $fileIn ) {
  print STDERR "Unable to open '$fileIn'.\n";
  exit 1;
}

if ( !open OUT, ">$fileOut" ) {
  print STDERR "Unable to open '$fileOut'.\n";
  exit 1;
}

while ( <IN> ) {
  my $newline;

  if ( /^(.*)ns1__(\w+\s+0?;.*)$/ ) {
#    print;
    $newline = $1 . $2 . "\n";
#    print $newline;
    $_ = $newline;
  }

if ( 0 ) {
  if ( /(^\s+\S+\s+)ns1__(.*)/ ) {
    $newline = $1 . $2 . "\n";
    if ( !/enum/ && /element$/) {
#      print;
#      print $newline;
      $_ = $newline;
    }
  } elsif ( /(^\s+unsigned long\*\s+)ns1__(.*)/ ) {
    $newline = $1 . $2 . "\n";
#    print $newline;
    $_ = $newline;
  } elsif ( /(^\s+std\:\:\S+\s+\>.*)ns1__(.*)/ ) {
    $newline = $1 . $2 . "\n";
    $_ = $newline;
  }
}

  print OUT;
}

if ( $inplace ) {
  system( "mv $fileIn $fileBack" );
  system( "mv $fileOut $fileIn" );
}
