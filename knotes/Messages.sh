#! /bin/sh
$EXTRACTRC *.rc settings/*.kcfg >> rc.cpp
$XGETTEXT `find . -name '*.h' -o -name '*.cpp'` -o $podir/knotes.pot
rm -f rc.cpp
