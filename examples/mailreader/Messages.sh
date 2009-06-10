#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.rc` >> rc.cpp
$XGETTEXT *.cpp -o $podir/mailreader.pot
rm -f *.cpp
