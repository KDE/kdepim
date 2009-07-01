#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.rc -o -name \*.ui` >> rc.cpp
$XGETTEXT *.cpp -o $podir/mailreader.pot
rm -f *.cpp
