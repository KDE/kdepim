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
  if ( $xml ) {
    if ( $_ =~ /(.*\<\/SOAP-ENV:Envelope\>)/ ) {
      printXml( $xml . $1 );
      $xml = "";
    } else {
      $xml .= $_;
    }
  } elsif ( $_ =~ /^(\<\?xml.*\?>)(.*)$/ ) {
    $xml = $1 . $2;

    if ( $xml =~ /(.*\<\/SOAP-ENV:Envelope\>)/ ) { 
      printXml( $1 );
      $xml = "";
      
    }
  }
}

sub printXml()
{
  $xml = shift;

  $xml =~ s/\n//g;
  $xml =~ s/\r//g;

  $out = "$in.$count.xml";

  print "Out: $out\n";

  if ( !open OUT, ">$out" ) {
    print STDERR "Unable to open file '$out'.\n";
  } else {
    print OUT $xml;
    close OUT;
  }

  $count += 1;
}
