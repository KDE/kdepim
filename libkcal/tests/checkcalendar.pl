#!/usr/bin/perl

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
    print "\n  FAILED: $error errors found.\n";
    if ( $error > 5 ) {
      system( "diff -u $file.ref $outfile" ); 
    }
    exit 1;
  } else {
    print "  OK\n";
  }
}
