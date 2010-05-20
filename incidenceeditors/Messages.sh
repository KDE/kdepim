#! /bin/sh
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT `find . -name "*.cpp"` -o $podir/libincidenceeditors.pot
rm -f rc.cpp
