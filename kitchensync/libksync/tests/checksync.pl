#!/usr/bin/perl

print "CHECK SYNC\n";

if ( @ARGV != 2 ) {
  print STDERR "Usage: checksync.pl <datadir> <refdir>\n";
  exit 1;
}

$dataDir = $ARGV[ 0 ];
$refDir = $ARGV[ 1 ];

print "DATADIR: $dataDir\n";
print "REFDIR: $refDir\n";

if ( !opendir DATA, $dataDir ) {
  print STDERR "Unable to open dir '$dataDir'.\n";
  exit 1;
}

$failed = 0;

@entries = readdir DATA;
foreach $entry ( @entries ) {
  if ( $entry !~ /(.*)\.test$/ ) { next; }
  
  $testId = $1;
  
  $testFile = "$dataDir/$entry";
  
  if ( !open TEST, $testFile ) {
    print STDERR "Unable to open test file '$testFile'.\n";
    exit 1;
  }
  $testTitle = <TEST>;
  close TEST;
  
  print "TEST $testId: $testTitle\n";

  $testFailed = 0;

  checkFile( "$dataDir/$testId.cal1.ics.out",
             "$refDir/$testId.cal1.ics.out.ref" );
  checkFile( "$dataDir/$testId.cal2.ics.out",
             "$refDir/$testId.cal2.ics.out.ref" );

  print "TEST $testId: ";
  if ( $testFailed ) {
    print "FAILED";
    $failed = 1;
  } else {
    print "OK";
  }
  print "\n";
}

closedir DATA;

if ( $failed ) { exit 1; }
else { exit 0; }

sub checkFile()
{
  $outFile = shift;
  $refFile = shift;
  
  if ( !open IN, $outFile ) {
    print STDERR "Unable to open file '$outFile'.\n";
    exit 1;
  }
  if ( !open REF, $refFile ) {
    print STDERR "Unable to open file '$refFile'.\n";
    exit 1;
  }

  while( <IN> ) {
    $lineIn = $_;
    $lineRef = <REF>;
    
    if ( $lineIn ne $lineRef ) {
      if ( $lineIn =~ /^DTSTAMP:/ && $lineRef =~ /^DTSTAMP:/ ) { next; }
      if ( $lineIn =~ /^CREATED:/ && $lineRef =~ /^CREATED:/ ) { next; }
      if ( $lineIn =~ /^LAST-MODIFIED:/ && $lineRef =~ /^LAST-MODIFIED:/ ) { next; }
      $testFailed = 1;
      print "EXPECTED: $lineRef";
      print "OUTPUT  : $lineIn";
    }
  }
}
