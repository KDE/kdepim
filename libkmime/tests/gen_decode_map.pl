#!/usr/bin/perl -w

use strict;

my @encodingMap = ( 'A' .. 'Z', 'a' .. 'z', '0' .. '9', '+', '/' );
my @decodingMap = (64) x 128;

my $len = scalar @encodingMap;
for ( my $i = 0 ; $i < $len ; $i++ ) {
    my $value = ord $encodingMap[$i];
    $decodingMap[$value] = $i;
}

for ( my $i = 0 ; $i < 128 ; $i += 16 ) {
    print "  ", join( ", ", @decodingMap[$i..($i+7)] ), ",  ",
                join( ", ", @decodingMap[($i+8)..($i+15)] ), ",\n";
}
