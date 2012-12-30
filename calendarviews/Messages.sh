#! /bin/sh
$EXTRACTRC *.ui *.kcfg >> rc.cpp
$XGETTEXT `find . -name "*.cpp"` -o $podir/libeventviews.pot
rm -f rc.cpp
