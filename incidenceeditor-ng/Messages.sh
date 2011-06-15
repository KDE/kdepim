#! /bin/sh
$EXTRACTRC `find . -name \*.ui -o -name \*.kcfg`  >> rc.cpp
$XGETTEXT `find . -name "*.cpp"` -o $podir/libincidenceeditors.pot
rm -f rc.cpp
