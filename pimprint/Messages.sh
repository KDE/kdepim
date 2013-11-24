#! /bin/sh
$EXTRACTRC `find . -name \*.ui -o -name \*.kcfg`  >> rc.cpp
$XGETTEXT `find . -name "*.cpp"` -o $podir/libpimprint.pot
rm -f rc.cpp
