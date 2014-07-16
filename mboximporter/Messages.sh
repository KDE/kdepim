#! /bin/sh
$EXTRACTRC ui/*.ui >> rc.cpp
$XGETTEXT `find . -name \*.cpp` -o $podir/mboximporter.pot
rm -rf rc.cpp
