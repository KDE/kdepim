#! /bin/sh
$EXTRACTRC ui/*.ui >> rc.cpp
$XGETTEXT `find -name '*.cpp'` -o $podir/pimsettingexporter.pot
rm -f rc.cpp

