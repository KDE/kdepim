#!/usr/bin/perl

if ( @ARGV != 1 ) {
  print STDERR "Usage: extractxml.pl <filename>\n";
  exit 1;
}

$in = $ARGV[ 0 ];

print "In: $in\n";

if ( !open IN, $in ) {
  print STDERR "Unable to open file '$in'.\n";
  exit 1;
}

$count = 1;

while ( <IN> ) {
  if ( $_ =~ /^(\<\?xml.*\<\/SOAP-ENV:Envelope\>)/ ) {
    $xml = $1;

    $out = "$in.$count.xml";

    print "Out: $out\n";

    if ( !open OUT, ">$out" ) {
      print STDERR "Unable to open file '$out'.\n";
    } else {
      print OUT $xml;
      close OUT;
    }
    
    $count += 1;
  } else {
#    print "XXXX: $_";
  }
}
