#!/usr/bin/perl

#    This file is part of libkcal.
#
#    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
#
#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.
#
#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; see the file COPYING.LIB.  If not, write to
#    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#    Boston, MA 02111-1307, USA.

if ( @ARGV != 1 ) {
  print STDERR "Missing arg: filename\n";
  exit 1;
}

$file = $ARGV[0];

$file =~ /^(.*)\.[^\.]*$/;

my $outfile = $file;
$outfile =~ /\/([^\/]*)$/;
$outfile = "$1.out";

$cmd = "./readandwrite $file $outfile";

#print "CMD $cmd\n";

if ( system( $cmd ) != 0 ) {
  print STDERR "Error running readandwrite\n";
  exit 1;
}

checkfile( $file, $outfile );

exit 0;

sub checkfile()
{
  my $file = shift;
  my $outfile = shift;

  print "Checking '$outfile':\n";

  my @ref;
  if ( !open( REF, "$file.ref" ) ) {
    print STDERR "Unable to open $file.ref\n";
    exit 1;
  }
  while( <REF> ) {
    push @ref, $_;
  }
  close REF;

  if ( !open( READ, $outfile ) ) {
    print STDERR "Unable to open $outfile\n";
    exit 1;
  }

  $error = 0;
  $i = 0;
  $line = 1;
  while( <READ> ) {
    $out = $_;
    $ref = @ref[$i++];

    if ( $out =~ /^DTSTAMP:[0-9ZT]+$/ && $ref =~ /^DTSTAMP:[0-9ZT]+$/ ) {
      next;
    }

    if ( $out ne $ref ) {
      $error++;
      print "  Line $line: Expected      : $ref";
      print "  Line $line: Actual output : $out";
    }
    
    $line++;
  }

  close READ;

  if ( $error > 0 ) {
    if ( -e "$file.fixme" ) {
      if ( !open( FIXME, "$file.fixme" ) ) {
        print STDERR "Unable to open $file.fixme\n";
        exit 1;
      }
      my $firstline = <FIXME>;
      $firstline =~ /^(\d+) known errors/;
      my $expected = $1;
      if ( $expected == $error ) {
        print "\n  EXPECTED FAIL: $error errors found.\n";
        print "    Fixme:\n";
        while( <FIXME> ) {
          print "      ";
          print;
        }
      } else {
        print "\n  UNEXPECTED FAIL: $error errors found, $expected expected.\n";
        exit 1;
      }
    } else {
      print "\n  FAILED: $error errors found.\n";
      if ( $error > 5 ) {
        system( "diff -u $file.ref $outfile" ); 
      }
      system( "touch FAILED" );
      exit 1;
    }
  } else {
    print "  OK\n";
  }
}
