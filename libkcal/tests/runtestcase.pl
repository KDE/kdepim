#!/usr/bin/perl

#    This file is part of libkcal.
#
#    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
#    Copyright (C) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>
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
#    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#    Boston, MA 02110-1301, USA.

# This little script runs a test program on a given (calendar) file and 
# compares the output to a reference file. All discrepancies are shown 
# to the user. Usage:
#      runtestcase.pl appname identifier testfile.ics
# 'identifier' is used as a suffix to allow multiple tests on the same input 
# file during a test run.
#
# The application/script appname is required to take two arguments:
#      appname inputfile outputfile
# where inputfile is the file to be used as input data, and the output of the
# program will go to outputfile (=testfile.ics.identifier.out if called through 
# runtestcase.pl). That outputfile is then compared to the reference file
# testfile.ics.ref.


if ( @ARGV != 3 ) {
  print STDERR "Missing arg! Arguments: testapp identifier filename \n";
  exit 1;
}

$app = $ARGV[0];
$id = $ARGV[1];
$file = $ARGV[2];

$MAXERRLINES=25;

$file =~ /^(.*)\.[^\.]*$/;

my $outfile = $file;
$outfile =~ /\/([^\/]*)$/;
$outfile = "$file.$id.out";

$cmd = "./$app $file $outfile 2> /dev/null";

#print "CMD $cmd\n";

if ( system( $cmd ) != 0 ) {
  print STDERR "Error running $app\n";
  exit 1;
}

checkfile( $file, $outfile );

exit 0;

sub checkfile()
{
  my $file = shift;
  my $outfile = shift;

  my $logentry = "Checking '$outfile':\n";

  my @ref;
  if ( !open( REF, "$file.$id.ref" ) ) {
    print STDERR "Unable to open $file.$id.ref\n";
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
  $line = 0;
  my $errorlines = 0;
  while( <READ> ) {
    $out = $_;
    $ref = @ref[$i++];
    $line++;

    # DTSTAMP, LAST-MODIFIED and CREATED might be different to the reference...
    if ( $out =~ /^DTSTAMP:[0-9ZT]+\r?$/ && $ref =~ /^DTSTAMP:[0-9ZT]+\r?$/ ) {
      next;
    }

    if ( $out =~ /^LAST-MODIFIED:[0-9ZT]+\r?$/ && $ref =~ /^LAST-MODIFIED:[0-9ZT]+\r?$/ ) {
      next;
    }

    if ( $out =~ /^CREATED:[0-9ZT]+\r?$/ && $ref =~ /^CREATED:[0-9ZT]+\r?$/ ) {
      next;
    }

    if ( $out ne $ref ) {
      if ( $errorlines == 0 ) {
        print $logentry;
      }
      $errorlines++;
      $error++;
      if ( $errorlines < $MAXERRLINES ) {
        print "  Line $line: Expected      : $ref";
        print "  Line $line: Actual output : $out";
      } elsif ( $errorlines == $MAXERRLINES ) {
        print "  <Remaining error suppressed>\n";
      }
    }
    
  }

  close READ;

  if ( $error > 0 ) {
    if ( -e "$file.$id.fixme" ) {
      if ( !open( FIXME, "$file.$id.fixme" ) ) {
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
        system( "diff -u $file.$id.ref $outfile" ); 
      }
      system( "touch FAILED" );
      exit 1;
    }
  } else {
    unlink($outfile);
#    print "  OK\n";
  }
}
